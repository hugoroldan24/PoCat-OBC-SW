// OBC Task serves as the central scheduler, coordinating the operation of all other tasks. 
// It is responsible for managing transitions between different operational modes, task scheduling, 
// power control, and essential satellite checkups.

/* ================= INCLUDES ================= */

#include <stddef.h>
#include <stdio.h> //mirar
#include <stdint.h> //mirar
#include "obc_manager.h"
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

//DEBUG
#include "semphr.h"


/* ================= MACROS AND CONSTANTS ================= */

// Task stack sizes
#define COMMS_STACK_SIZE    3000
#define PAYLOAD_STACK_SIZE	4000 // ??
#define EPS_STACK_SIZE		250 // ??
#define OBDH_STACK_SIZE     1000 // ??
#define ADCS_STACK_SIZE     1000 // ??
#define HEALTH_STACK_SIZE   250 // ??



// Task priorities
#define COMMS_PRIORITY           0u
#define PAYLOAD_PRIORITY         2 // ?? Mirar!!
#define EPS_PRIORITY             2 // ?? 
#define OBDH_PRIORITY            5 // ??
#define ADCS_PRIORITY            5 // ??
#define HEALTH_PRIORITY           5

/* ================= TYPE DEFINITIONS ================= */

/* Satellite modes */
typedef enum {
    OBC_STATE_NOMINAL = 0u,
    OBC_STATE_CONTINGENCY,
    OBC_STATE_SUN_SAFE,
    OBC_STATE_COMMISSIONING
} OBC_SatelliteState_t;

/* ================= GLOBAL VARIABLES ================= */
extern TaskHandle_t obc_task_handle;

//DEBUG
SemaphoreHandle_t uart_mutex;

/* ================= MODULE-LEVEL VARIABLES ================= */
static const EVT_StateHandlers_t *event_handlers;
static OBC_SatelliteState_t currentState;
static TaskHandle_t payload_task_handle, eps_task_handle, comms_task_handle, obdh_task_handle, adcs_task_handle, health_task_handle;
static TaskNotifyValue_t notifyVal;

/* ================= PRIVATE FUNCTION PROTOTYPES ================= */

static void create_tasks(void);
static void setup_obc(void);
static void process_obc(OBC_SatelliteState_t *currentState);
static ReturnCode_t DispatchStateHandler(const EVT_StateHandlers_t *handlers, OBC_SatelliteState_t state);

// static void create_queues(void);  // TODO: implement this function

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */

void obc_task(void *pv_parameters) 
{
    setup_obc();

    for (;;) {
        process_obc(&currentState);
    }
}

ReturnCode_t Notify_OBC_From_Task(OBC_TaskID_t id, uint32_t val)
{
    ReturnCode_t retVal = OK;

    if(id >= NUM_TASKS)
    {
        retVal = NOT_OK;
    }
    
    const EVT_Config_t  *cfg;
    
    if(EVT_GetConfig(id, &cfg) != OK)
    {
        retVal = NOT_OK;
    }

    uint32_t encoded_val = 0u;

    encoded_val = EVT_SET_FIELD(val, cfg->mask, cfg->offset);

    BaseType_t ret = xTaskNotify(obc_task_handle, encoded_val, eSetBits);

    if(ret != pdPASS)
    {
        retVal = NOT_OK;
    }

    return retVal; 
}


/* ================= PRIVATE FUNCTION DEFINITIONS ================= */

static void setup_obc(void) {

    printf("Setting up OBC...\n");
    // 1. Create queues
    // create_queues();  // TODO: implement this function

    // 2. Create tasks
    create_tasks();

    //Debug sempahore for accessing UART for logging
    uart_mutex = xSemaphoreCreateMutex();
}

static void create_tasks(void) 
{
    (void) xTaskCreate(payload_task, "PAYLOAD", PAYLOAD_STACK_SIZE, NULL, PAYLOAD_PRIORITY, &payload_task_handle);
    (void) xTaskCreate(eps_task,     "EPS",     EPS_STACK_SIZE, NULL, EPS_PRIORITY,         &eps_task_handle);
    (void) xTaskCreate(comms_task,   "COMMS",   COMMS_STACK_SIZE, NULL, COMMS_PRIORITY,     &comms_task_handle);
    (void) xTaskCreate(obdh_task,    "OBDH",    OBDH_STACK_SIZE, NULL, OBDH_PRIORITY,       &obdh_task_handle);    
    (void) xTaskCreate(adcs_task,    "ADCS",    ADCS_STACK_SIZE, NULL, ADCS_PRIORITY,       &adcs_task_handle);   
    (void) xTaskCreate(health_task,  "HEALTH",  HEALTH_STACK_SIZE, NULL, HEALTH_PRIORITY,   &health_task_handle);    
}

static void process_obc(OBC_SatelliteState_t *currentState) 
{

    printf("Processing OBC...\n");

    // Finish implementing... Need to know how the rest of the tasks work...

    /* We iterate until no actions to be done are available */
   
    waitForNotification(&notifyVal);

    (void)EVT_ResolveHandlersFromNotify(&event_handlers, notifyVal);

    (void)DispatchStateHandler(event_handlers, *currentState);
}

static ReturnCode_t DispatchStateHandler(const EVT_StateHandlers_t *handlers, OBC_SatelliteState_t state)
{
    ReturnCode_t retVal = NOT_OK;

    if(handlers == NULL)
    {
        return retVal;
    }
    switch (state)
    {
        case OBC_STATE_NOMINAL:
            if(handlers->on_nominal != NULL)
            {
                retVal = OK;
                handlers->on_nominal();
            }
            break;

        case OBC_STATE_CONTINGENCY:
            if(handlers->on_contingency != NULL)
            {
                retVal = OK;
                handlers->on_contingency();
            }
            break;

        case OBC_STATE_SUN_SAFE:
            if(handlers->on_sun_safe != NULL)
            {
                retVal = OK;
                handlers->on_sun_safe();
            }
            break;

        case OBC_STATE_COMMISSIONING:
            if(handlers->on_commissioning != NULL)
            {    
                retVal = OK;
                handlers->on_commissioning();
            }
            
            break;
        default:
            /* Manage undefined state */
            break;
    }
    return retVal;
}
