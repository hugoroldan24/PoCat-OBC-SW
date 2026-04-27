#include "log.h"
#include <string.h>

//DEBUG
#include "obc_manager.h"
#include "semphr.h"

extern SemaphoreHandle_t uart_mutex;

static UART_HandleTypeDef huart3;

int _write(int file, char *ptr, int len)
{
    xSemaphoreTake(uart_mutex, portMAX_DELAY);
    HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    xSemaphoreGive(uart_mutex);

    return len;
}






