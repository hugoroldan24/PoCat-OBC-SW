/* ================= INCLUDES ================= */
#include "comms_task.h"
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
    TaskNotifyValue_t value;
        
    //wait_for_notification(&value);
    
    /* Tiene acceso al LoRa */

    /* Leer a través del LoRa */


}
