/* ================= INCLUDES ================= */
#include <stdio.h>
#include "common.h"
#include "evt_service.h"
#include "obc_manager.h"

/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void health_setup(void);
static void health_process(void);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */

void health_task(void *pv_parameters){
    /*TODO implementation*/
    health_setup();

    // PROCESS STAGE

    //  it does not have different performances depending on the mode

    for (;;)
    {
        health_process();
    }
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void health_setup(void)
{
    printf("Setting up HEALTH...\n");
    // Apply the default configuration
}

static void health_process(void)
{
    EVT_ID_t evt_id[EVT_EVENTS_PER_SLOT] = {EVT_UNDEF};

    evt_id[0] = EVT_HEALTH_TO_COMMISSIONING; 

    (void)obc_send_event_from_task(TASK_HEALTH_ID, evt_id, 1);

    vTaskDelay(pdMS_TO_TICKS(500));
}