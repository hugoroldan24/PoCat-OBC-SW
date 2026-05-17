#ifndef INC_LOGGING_H
#define INC_LOGGING_H

#include "main.h"

HAL_StatusTypeDef UART7_SendString(const char *str);

#define LOGGING(str) UART7_SendString(str)
 
#endif