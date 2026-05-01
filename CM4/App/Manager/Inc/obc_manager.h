#ifndef INC_OBC_MANAGER_H
#define INC_OBC_MANAGER_H


#include <stdint.h>
#include "evt_service.h"


typedef enum {
   OBC_TASK_PAYLOAD_ID = 0,
   OBC_TASK_COMMS_ID,
   OBC_TASK_EPS_ID,
   OBC_TASK_OBDH_ID,
   OBC_TASK_ADCS_ID,
   OBC_TASK_HEALTH_ID,
   OBC_NUM_TASKS
} OBC_TaskID_t

typedef struct {
    EVT_ID_t id;
    void (*on_commissioning)(void);
    void (*on_nominal)(void);
    void (*on_sun_safe)(void);
    void (*on_contingency)(void);
} OBC_StateHandlers_t;

ReturnCode_t obc_init_task();
ReturnCode_t obc_send_event_from_task(OBC_TaskID_t id, uint32_t *val, uint32_t len)


#endif