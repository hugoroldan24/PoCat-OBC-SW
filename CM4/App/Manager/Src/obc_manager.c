// OBC Task serves as the central scheduler, coordinating the operation of all other tasks. 
// It is responsible for managing transitions between different operational modes, task scheduling, 
// power control, and essential satellite checkups.

/* ================= INCLUDES ================= */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h> //mirar
#include <stdint.h> //mirar
#include <string.h>
#include "obc_manager.h"
#include "obc_event_routing.h"
#include "FreeRTOS.h" // mirar
#include "task.h" // mirar
#include "main.h"
#include "eps.h"
#include "comms.h"
#include "obdh.h"
#include "payload.h"
#include "common.h"
#include "adcs.h"
#include "health.h"
#include "evt_core.h"
#include "evt_tables.h"


/* ================= MACROS AND CONSTANTS ================= */


#define OBC_NUM_SLOTS (3u)

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

typedef enum{
    SLOT_UNINITIALIZED = (0u),
    SLOT_FILLED        = (1u),
    SLOT_PROCESSED     = (2u)
} OBC_SlotStatus_t;

/* Satellite modes */
typedef enum {
    OBC_STATE_NOMINAL = 0u,
    OBC_STATE_CONTINGENCY,
    OBC_STATE_SUN_SAFE,
    OBC_STATE_COMMISSIONING
} OBC_SatelliteState_t;

typedef struct{
    uint32_t value,
    OBC_SlotStatus_t status
} OBC_EventSlot_t;

typedef struct {
    EVT_ID_t id;
    void (*on_commissioning)(void);
    void (*on_nominal)(void);
    void (*on_sun_safe)(void);
    void (*on_contingency)(void);
} OBC_StateHandlers_t;

/* ================= GLOBAL VARIABLES ================= */
TaskHandle_t obc_task_handle;
TaskHandle_t payload_task_handle;
TaskHandle_t eps_task_handle;
TaskHandle_t comms_task_handle;
TaskHandle_t obdh_task_handle;
TaskHandle_t adcs_task_handle;
TaskHandle_t health_task_handle;

OBC_EventSlot_t slots[OBC_NUM_SLOTS] = {0};

/* ================= MODULE-LEVEL VARIABLES ================= */
static const OBC_StateHandlers_t *event_handlers;
static OBC_SatelliteState_t obc_current_state = OBC_STATE_NOMINAL;

static TaskNotifyValue_t num_slots_used;

static EVT_Decoded_t decoded_events[EVT_MAX_FIELDS];
static uint32_t num_events;

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
static void obc_task(void *pv_parameters) 
static ReturnCode_t create_tasks(void);
static ReturnCode_t setup_obc(void);
static ReturnCode_t process_obc();
static ReturnCode_t process_slot(OBC_Slot_t *slot);
static ReturnCode_t obc_execute_state_handler(const OBC_StateHandlers_t *handlers);

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

ReturnCode_t obc_send_event_from_task(OBC_TaskID_t id, uint32_t *val, uint32_t len)
{

    if ((id >= OBC_NUM_TASKS) || (val == NULL) || (len >= OBC_NUM_SLOTS))
    {
       return NOT_OK;
    } 

    EVT_Type_t type = obc_to_evt_map[id];
    uint32_t l_slot[OBC_NUM_SLOTS];

    for(uint32_t i; i < len; i++)
    {
        evt_encode(type, val[i], &l_slot[i]);
    }

    taskENTER_CRITICAL(); 
    for(uint32_t i; i < len; i++)
    {
        slots[i].value |= l_slot[i];
        slots[i].status = SLOT_FILLED;
    }
    taskEXIT_CRITICAL();

    
// THIS GAP IS DONE SO IF PREEMPTION OCCURS HERE WE CAN STILL ADD MORE EVENTS 

    taskENTER_CRITICAL();
           
    if(slots[0].status == SLOT_FILLED)
    {
        //No need to assert, when using eNoAction returns always pdPASS 
        xTaskNotify(obc_task_handle, 0, eNoAction);   
    }        
        
    taskEXIT_CRITICAL();

    return OK;
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void obc_task(void *pv_parameters) 
{
    setup_obc();

    for (;;) {
        process_obc();
    }
}

static void setup_obc(void) {

    printf("Setting up OBC...\n");
    // 1. Create queues
    // create_queues();  // TODO: implement this function

    create_subsystem_tasks();

}

static void create_subsystem_tasks(void) 
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

static void process_obc() 
{
    uint32_t l_slots[OBC_NUM_SLOTS];

    printf("Processing OBC...\n");

    /* We iterate until no actions to be done are available */
   
    wait_for_notification(NULL); 

    for(uint32_t i=0; i<OBC_NUM_SLOTS; i++)
    {
        if(slots[i].status == SLOT_FILLED)
        {
           (void)evt_decode(slots[i].value, &decoded_events, &num_events);
 
           for(uint32_t j=0u; j < num_events; j++)
           {
               event_handlers = obc_resolve_event_handler(&decoded_events[j]);
               (void)obc_execute_state_handler(event_handlers);
               slots[i].status = SLOT_PROCESSED;
           }
        }
        
    } 
}

static ReturnCode_t obc_execute_state_handler(const OBC_StateHandlers_t *handlers)
{

    ReturnCode_t (*state_handler)(void) = NULL;

    if(handlers == NULL)
    {
        return NOT_OK;
    }
    switch (obc_current_state)
    {
        case OBC_STATE_NOMINAL:
            state_handler = handlers->on_nominal;
            break;

        case OBC_STATE_CONTINGENCY:
            state_handler = handlers->on_contingency;
            break;

        case OBC_STATE_SUN_SAFE:
            state_handler = handlers->on_sun_safe;
            break;

        case OBC_STATE_COMMISSIONING:
            state_handler = handlers->on_commissioning;
            break;
        default:
            /* Manage undefined state */
            return NOT_OK;
            break;
    }

    if(state_handler == NULL)
    {
        return NOT_OK;
    }
    
    return state_handler();
}

static ReturnCode_t process_all_slots(OBC_Slot_t *slots)
{
    ReturnCode_t ret_val = OK;
    for (uint32_t i = 0u; i < OBC_NUM_SLOTS; i++)
    {
        ret_val = process_slot(&slots[i]);
        if(ret_val == NOT_OK)
        {
            return ret_val;
        }
    }
    return OK;
}

static ReturnCode_t process_slot(OBC_Slot_t *slot)
{
    if (slot->status != SLOT_FILLED)
    {
        return NOT_OK;
    }

    OBC_Event_t decoded_events[MAX_EVENTS];
    uint32_t num_events = 0u;

    (void)evt_decode(slot->value, decoded_events, &num_events);

    for (uint32_t i = 0u; i < num_events; i++)
    {
        OBC_EventHandler_t handler = obc_resolve_event_handler(&decoded_events[j]);
        if(handler == NULL)
        {
            return NOT_OK;
        }
        (void)obc_execute_state_handler(handler);
    }

    slot->status = SLOT_PROCESSED;
    return OK;
}
