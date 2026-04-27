/* ================= INCLUDES ================= */
#include <stdio.h>
/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void setup_health(void);
static void process_health(void);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */

void health_task(void *pv_parameters){
    /*TODO implementation*/
    setup_health();

    // PROCESS STAGE

    //  it does not have different performances depending on the mode

    for (;;)
    {
        process_health();
    }
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void setup_health(void)
{
    printf("Setting up HEALTH...\n");
    // Apply the default configuration
}

static void process_health(void)
{
    printf("Processing HEALTH...\n");
}