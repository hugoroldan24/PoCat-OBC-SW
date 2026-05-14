/* ================= INCLUDES ================= */
#include "comms_task.h"
#include "evt_service.h"
#include "common.h"
#include "obc_manager.h"
#include <stdio.h>


/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void setup_comms(void);
static void process_comms(void);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void comms_task(void *pv_parameters){
    setup_comms();

    for(;;)
    {
        process_comms();
    }
    /*TODO implementation*/
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void setup_comms(void)
{
    printf("Setting up COMMS...\n");
    // Apply the default configuration
}



static void process_comms(void)
{

    EVT_ID_t evt_id[EVT_EVENTS_PER_SLOT] = {EVT_UNDEF};

    evt_id[0] = EVT_COMMS_TO_CONTINGENCY; 

    (void)obc_send_event_from_task(TASK_COMMS_ID, evt_id, 1);

    vTaskDelay(pdMS_TO_TICKS(500));
    /* Leer a través del LoRa */
}
