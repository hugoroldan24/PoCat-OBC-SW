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
#define PAYLOAD_PRIORITY         1 // ?? Mirar!!
#define EPS_PRIORITY             2 // ?? 
#define OBDH_PRIORITY            3 // ??
#define ADCS_PRIORITY            4 // ??
#define HEALTH_PRIORITY          5

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
SemaphoreHandle_t log_smph;

/* ================= MODULE-LEVEL VARIABLES ================= */
static const EVT_StateHandlers_t *event_handlers;
static OBC_SatelliteState_t currentState = OBC_STATE_NOMINAL;
static TaskHandle_t payload_task_handle, eps_task_handle, comms_task_handle, obdh_task_handle, adcs_task_handle, health_task_handle;
static TaskNotifyValue_t notifyVal;

static const EVT_Type_t obc_to_evt_map[NUM_TASKS] = {
    [TASK_PAYLOAD_ID] = EVT_TYPE_PAYLOAD,
    [TASK_COMMS_ID]   = EVT_TYPE_TC,
    [TASK_EPS_ID]     = EVT_TYPE_EPS,
    [TASK_OBDH_ID]    = EVT_TYPE_OBDH,
    [TASK_ADCS_ID]    = EVT_TYPE_ADCS,
    [TASK_HEALTH_ID]  = EVT_TYPE_HEALTH
};

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

ReturnCode_t OBC_SubmitEvent(OBC_TaskID_t id, uint32_t val)
{
    const EVT_Config_t *cfg = NULL;

    if (id >= NUM_TASKS)
    {
       return NOT_OK;
    }

    EVT_Type_t type = obc_to_evt_map[id];

    cfg = EVT_GetConfig(type);

    if (cfg == NULL)
    {
       return NOT_OK;
    }

    uint32_t encoded_val = EVT_SET_FIELD(type, EVT_MASK_TYPE, EVT_OFFSET_TYPE) |
                           EVT_SET_FIELD(val, cfg->mask, cfg->offset);
    

    if (xTaskNotify(obc_task_handle, encoded_val, eSetBits) != pdPASS)
    {
       return NOT_OK;
    }     

    return OK;
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */

static void setup_obc(void) {

    printf("Setting up OBC...\n");
    // 1. Create queues
    // create_queues();  // TODO: implement this function

    // 2. Create tasks
    create_tasks();

    //Debug sempahore for accessingfor logging
    log_smph = xSemaphoreCreateMutex();
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

