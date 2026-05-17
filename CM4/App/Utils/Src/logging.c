/* ================= INCLUDES ================= */

#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>


/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */

extern UART_HandleTypeDef huart7;
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
/* ================= PUBLIC FUNCTION DEFINITIONS ================= */


HAL_StatusTypeDef UART7_SendString(const char *str)
{
    if (str == NULL)
    {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart7,
                                            (uint8_t *)str,
                                            strlen(str),
                                            HAL_MAX_DELAY);
    return status;
}


/* ================= PRIVATE FUNCTION DEFINITIONS ================= */