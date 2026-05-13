// OBC Task serves as the central scheduler, coordinating the operation of all other tasks. 
// It is responsible for managing transitions between different operational modes, task scheduling, 
// power control, and essential satellite checkups.

/* LEDS */
/*PG8: 11 ->NOMINAL
PI5:12     -> COMMISSIONING
 PG15:15   -> SUN SAFE
 PF1:16 -> CONTINGENCY */


/* EN LA DK2 */
/*PF8: MISO: 21  */
/*PF9: MOSI:19 */
/*PF7: SCK: 23  */
/*PG2: NSS 29*/ /* Usado para el testing de tareas y leds también, luego al cambiar al LoRa tenerlo en cuenta!!!*/

/* ================= INCLUDES ================= */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h> //mirar
#include <stdint.h> //mirar
#include <string.h>
#include "obc_manager.h"
#include "obc_event_mapping.h"
#include "FreeRTOS.h" // mirar
#include "task.h" // mirar
#include "main.h"
#include "eps_task.h"
#include "comms_task.h"
#include "obdh_task.h"
#include "payload_task.h"
#include "common.h"
#include "adcs_task.h"
#include "health_task.h"


/* ================= MACROS AND CONSTANTS ================= */
// Task stack sizes
#define OBC_STACK_SIZE     3000
#define COMMS_STACK_SIZE   3000 // ??
#define PAYLOAD_STACK_SIZE 4000 // ??
#define EPS_STACK_SIZE	   250  // ??
#define OBDH_STACK_SIZE    1000 // ??
#define ADCS_STACK_SIZE    1000 // ??
#define HEALTH_STACK_SIZE  250  // ??


// Task priorities
#define OBC_PRIORITY       6 // ??
#define COMMS_PRIORITY     0 // ??
#define PAYLOAD_PRIORITY   1 // ?? Mirar!!
#define EPS_PRIORITY       2 // ?? 
#define OBDH_PRIORITY      3 // ??
#define ADCS_PRIORITY      4 // ??
#define HEALTH_PRIORITY    5

/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
TaskHandle_t obc_task_handle;
TaskHandle_t payload_task_handle;
TaskHandle_t eps_task_handle;
TaskHandle_t comms_task_handle;
TaskHandle_t obdh_task_handle;
TaskHandle_t adcs_task_handle;
TaskHandle_t health_task_handle;

OBC_SatelliteState_t obc_current_state = OBC_STATE_NOMINAL;

/* ================= MODULE-LEVEL VARIABLES ================= */
static TaskNotifyValue_t obc_event_slot;

static const OBC_Handler_t *event_handlers;


static EVT_Decoded_t decoded_events={0};

/*==============================================================================
 * FreeRTOS Static Task Control Blocks and Stacks
 *
 * This section defines all statically allocated resources required for
 * FreeRTOS task creation using xTaskCreateStatic().
 *
 * For each task, the following are defined:
 *   - StaticTask_t: Task Control Block (TCB)
 *   - StackType_t[]: Task stack memory
 *
 * These objects must have static storage duration to ensure they remain
 * valid for the entire lifetime of the RTOS scheduler.
 *
 * NOTE:
 * - Stack sizes are expressed in units of StackType_t (not bytes)
 * - Memory is fully statically allocated (no heap usage for tasks)
 *==============================================================================*/

static StaticTask_t obc_task_tcb;
static StackType_t  obc_task_stack[OBC_STACK_SIZE];

static StaticTask_t payload_task_tcb;
static StackType_t  payload_task_stack[PAYLOAD_STACK_SIZE];

static StaticTask_t eps_task_tcb;
static StackType_t  eps_task_stack[EPS_STACK_SIZE];

static StaticTask_t comms_task_tcb;
static StackType_t  comms_task_stack[COMMS_STACK_SIZE];

static StaticTask_t obdh_task_tcb;
static StackType_t  obdh_task_stack[OBDH_STACK_SIZE];

static StaticTask_t adcs_task_tcb;
static StackType_t  adcs_task_stack[ADCS_STACK_SIZE];

static StaticTask_t health_task_tcb;
static StackType_t  health_task_stack[HEALTH_STACK_SIZE];



/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void obc_task(void *pv_parameters);
static ReturnCode_t obc_create_tasks(void);
static ReturnCode_t obc_setup(void);
static ReturnCode_t obc_process(void);
static ReturnCode_t obc_execute_handler(const OBC_Handler_t *handler);
static ReturnCode_t obc_process_slot(TaskNotifyValue_t slot);

// static void create_queues(void);  // TODO: implement this function

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */

ReturnCode_t obc_init_task()
{
    obc_task_handle = xTaskCreateStatic(obc_task, "OBC", OBC_STACK_SIZE, NULL, OBC_PRIORITY, obc_task_stack, &obc_task_tcb);
    if(obc_task_handle == NULL)
    {
        return NOT_OK;
    }
    return OK;
}

ReturnCode_t obc_send_event_from_task(OBC_TaskID_t id, uint32_t *val, uint32_t num_events)
{

    if ((id >= OBC_NUM_TASKS) || (val == NULL))
    {
       return NOT_OK;
    } 

    EVT_Type_t type = obc_to_evt_map[id];

    TaskNotifyValue_t notif_val = 0u;
    (void)evt_encode(type, val, num_events, &notif_val);
    
    if(xTaskNotify(obc_task_handle, notif_val, eSetValueWithoutOverwrite) != pdPASS)
    {
        xtaskENTER_CRITICAL();
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET); /*Activate LED because more than one task were able to notify OBC task before he woke up*/
        xtaskEXIT_CRITICAL();
        return NOT_OK;
    }
    
    return OK;
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void obc_task(void *pv_parameters) 
{
    obc_setup();

    for (;;) {
        obc_process();
    }
}

static void obc_setup(void) {

    printf("Setting up OBC...\n");
    // 1. Create queues
    // create_queues();  // TODO: implement this function

    obc_create_subsystem_tasks();

    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8, GPIO_PIN_SET);

}

static ReturnCode_t obc_create_subsystem_tasks(void) 
{
   payload_task_handle = xTaskCreateStatic(payload_task, "PAYLOAD", PAYLOAD_STACK_SIZE, NULL, PAYLOAD_PRIORITY, payload_task_stack, &payload_task_tcb);
   if (payload_task_handle == NULL) return NOT_OK;

   eps_task_handle     = xTaskCreateStatic(eps_task, "EPS", EPS_STACK_SIZE, NULL, EPS_PRIORITY, eps_task_stack, &eps_task_tcb); 
   if (eps_task_handle == NULL) return NOT_OK;

   comms_task_handle   = xTaskCreateStatic(comms_task, "COMMS", COMMS_STACK_SIZE, NULL, COMMS_PRIORITY, comms_task_stack, &comms_task_tcb);
   if (comms_task_handle == NULL) return NOT_OK;

   obdh_task_handle    = xTaskCreateStatic(obdh_task, "OBDH", OBDH_STACK_SIZE, NULL, OBDH_PRIORITY, obdh_task_stack, &obdh_task_tcb);
   if (obdh_task_handle == NULL) return NOT_OK;

   adcs_task_handle    = xTaskCreateStatic(adcs_task, "ADCS", ADCS_STACK_SIZE, NULL, ADCS_PRIORITY, adcs_task_stack, &adcs_task_tcb);
   if (adcs_task_handle == NULL) return NOT_OK;

   health_task_handle  = xTaskCreateStatic(health_task, "HEALTH", HEALTH_STACK_SIZE, NULL, HEALTH_PRIORITY, health_task_stack, &health_task_tcb);
   if (adcs_task_handle == NULL) return NOT_OK;

   return OK;
}

static void obc_process(void)
{
    
    printf("Processing OBC...\n");

    /* We iterate until no actions to be done are available */
   
    wait_for_notification(&obc_event_slot); 


    (void)obc_process_slot(obc_event_slot);
}

static ReturnCode_t obc_execute_handler(const OBC_Handler_t *handler)
{
    if(handler->event_handler == NULL)
    {
        return NOT_OK;
    }
    
    return handler->event_handler(obc_current_state);
}

static ReturnCode_t obc_process_slot(TaskNotifyValue_t slot)
{
   uint32_t num_events = 0u;

   (void)evt_decode(slot, &decoded_events, &num_events);

   for (uint32_t i = 0u; i < num_events; i++)
   {
        const OBC_Handler_t *handler = obc_resolve_event_handler(decoded_events.id[i], decoded_events.type);
        if(handler == NULL)
        {
            return NOT_OK;
        }
        (void)obc_execute_handler(handler);
    }

    return OK;
}
