// The ADCS software processes sensor data, executes algorithms to determine the satellite's 
// position, and issues commands to control and stabilize its orientation. This ensures that 
// the satellite can reliably carry out mission objectives such as precise data collection, 
// alignment with targets, and consistent communication.

/* ================= INCLUDES ================= */
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "obc_manager.h"
#include "adcs.h"

/* ================= MACROS AND CONSTANTS ================= */
#define ADCS_DETUMBLING_MODE (1 << 0)
#define ADCS_NADIR_POINTING_MODE (1 << 1)

/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void setup_adcs(void);
static void process_adcs(void);
static void detumble(void);
static void point_to_nadir(void);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void adcs_task(void *pv_parameters) {
    setup_adcs();
    
    for(;;)
    {
        process_adcs();
    }

}
/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void setup_adcs(void) {
    // Apply the default configuration
}

static void process_adcs(void) {
    TaskNotifyValue_t value;
    waitForNotification(&value);

    if (value & ADCS_DETUMBLING_MODE) {
        detumble();
    }
    if (value & ADCS_NADIR_POINTING_MODE) {
        point_to_nadir();
    }
}


static void detumble(void) {

    // Don't exit function until finished 

}

static void point_to_nadir(void) {

    // Don't exit function until finished 

}

