/* ================= INCLUDES ================= */
#include <stdio.h> //mirar
#include "FreeRTOS.h"
#include "common.h"
#include "main.h"
#include "payload_task.h"

/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void payload_setup(void);
static void payload_process(void);
static void payload_capture_temperature(void);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void payload_task(void *pv_parameters) {
    
    payload_setup();

    for (;;) {
        payload_process();
    }

}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void payload_setup(void) {

    // Apply default configuration
    // ...
    printf("Setting up PAYLOAD...\n");
    
}

static void payload_process(void) {

    TaskNotifyValue_t value;
    wait_for_notification(&value);

    if (value & PAYLOAD_TEMP_CAPTURE) {
        payload_capture_temperature();
    }
    // if ... (not else if!!)

}


static void payload_capture_temperature(void) {

    // /* 1. Initialize temperature sensor with current settings */
    // initSens(huart4, resolution, compressibility, info);

    // /* 2. Take the temperature into ‘info’ buffer */
    // getTemperature(huart4, info);


}

