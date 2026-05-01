/* ================= INCLUDES ================= */
#include "common.h"
#include "FreeRTOS.h"
#include "task.h"
/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void wait_for_notification(TaskNotifyValue_t* value) 
{
    (void) xTaskNotifyWait( 0u,          // don't clear on entry
                    MASK_32BIT, // clear all bits on exit
                    value,
                    portMAX_DELAY );
}
/* ================= PRIVATE FUNCTION DEFINITIONS ================= */

