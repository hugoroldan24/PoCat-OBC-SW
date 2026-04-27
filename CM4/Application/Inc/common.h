#ifndef INC_COMMON_H
#define INC_COMMON_H

#include <stdint.h>

#define MASK_32BIT (0xFFFFFFFFUL)

typedef uint32_t TaskNotifyValue_t;

typedef enum{
    OK = 0u,
    NOT_OK
} ReturnCode_t;

void waitForNotification(TaskNotifyValue_t* value);


#endif