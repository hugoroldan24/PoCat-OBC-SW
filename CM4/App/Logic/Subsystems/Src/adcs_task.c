// The ADCS software processes sensor data, executes algorithms to determine the satellite's 
// position, and issues commands to control and stabilize its orientation. This ensures that 
// the satellite can reliably carry out mission objectives such as precise data collection, 
// alignment with targets, and consistent communication.

/* ================= INCLUDES ================= */
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "obc_manager.h"
#include "adcs_task.h"
#include "evt_service.h"


/* ================= MACROS AND CONSTANTS ================= */
#define ADCS_DETUMBLING_MODE (1 << 0)
#define ADCS_NADIR_POINTING_MODE (1 << 1)

/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void adcs_setup(void);
static void adcs_process(void);
static void adcs_detumble(void);
static void adcs_point_to_nadir(void);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void adcs_task(void *pv_parameters) {
    adcs_setup();
    
    for(;;)
    {
        adcs_process();
    }

}
/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void adcs_setup(void) {
    // Apply the default configuration
}

static void adcs_process(void) {
    
    /*
    TaskNotifyValue_t value;
    wait_for_notification(&value);

    if (value & ADCS_DETUMBLING_MODE) {
        adcs_detumble();
    }
    if (value & ADCS_NADIR_POINTING_MODE) {
        adcs_point_to_nadir();
    }
        */
    EVT_ID_t evt_id[EVT_EVENTS_PER_SLOT] = {EVT_UNDEF};

    evt_id[0] = EVT_ADCS_TO_NOMINAL; 

    (void)obc_send_event_from_task(TASK_ADCS_ID, evt_id, 1);

    vTaskDelay(pdMS_TO_TICKS(500));

}


static void adcs_detumble(void) {

    // Don't exit function until finished 

}

static void adcs_point_to_nadir(void) {

    // Don't exit function until finished 

}

