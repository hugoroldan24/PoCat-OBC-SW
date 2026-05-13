/* ================= INCLUDES ================= */
#include <stdio.h>
#include "common.h"

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
    printf("Processing HEALTH...\n");
    TaskNotifyValue_t value;
    wait_for_notification(&value);
}