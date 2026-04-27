/* ================= INCLUDES ================= */
#include <stdio.h> //mirar
#include "FreeRTOS.h"
#include "common.h"
#include "main.h"
#include "payload.h"

/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void setup_payload(void);
static void process_payload(void);
static void capture_temperature(void);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void payload_task(void *pv_parameters) {
    
    setup_payload();

    for (;;) {
        process_payload();
    }

}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void setup_payload(void) {

    // Apply default configuration
    // ...
    
}

static void process_payload(void) {

    TaskNotifyValue_t value;
    waitForNotification(&value);

    if (value & PAYLOAD_TEMP_CAPTURE) {
        capture_temperature();
    }
    // if ... (not else if!!)

}


static void capture_temperature(void) {

    // /* 1. Initialize temperature sensor with current settings */
    // initSens(huart4, resolution, compressibility, info);

    // /* 2. Take the temperature into ‘info’ buffer */
    // getTemperature(huart4, info);

    printf("Capturing temperature...\n");

}

