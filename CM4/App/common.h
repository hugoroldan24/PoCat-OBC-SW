#ifndef INC_COMMON_H
#define INC_COMMON_H

#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

#define MASK_32BIT (0xFFFFFFFFUL)

typedef uint32_t TaskNotifyValue_t;

typedef enum {
    OK = 0u,
    NOT_OK
} ReturnCode_t;

void wait_for_notification(TaskNotifyValue_t* value);


#endif