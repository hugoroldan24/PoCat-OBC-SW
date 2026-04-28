#ifndef INC_OBC_MANAGER_H_
#define INC_OBC_MANAGER_H_

#include <stdint.h>
#include "common.h"

typedef enum {
    TASK_PAYLOAD_ID     = 0u,
    TASK_COMMS_ID,
    TASK_EPS_ID,
    TASK_OBDH_ID,
    TASK_ADCS_ID,
    TASK_HEALTH_ID,

    NUM_TASKS
} OBC_TaskID_t;



// TODO: revisar stack sizes y prioridades!


// Task stack sizes
#define OBC_STACK_SIZE			1000 // ??


// Task priorities
#define OBC_PRIORITY             7 // ??


 
/**
 * @brief Communications task function, it runs the OBC state machine.
 */
void obc_task(void *pv_parameters);

ReturnCode_t OBC_SubmitEvent(OBC_TaskID_t id, uint32_t val);

#endif /* INC_OBC_H_ */