#ifndef INC_OBC_MANAGER_H
#define INC_OBC_MANAGER_H


#include <stdint.h>
#include "evt_service.h"


typedef enum {
   TASK_PAYLOAD_ID = 0,
   TASK_COMMS_ID,
   TASK_EPS_ID,
   TASK_OBDH_ID,
   TASK_ADCS_ID,
   TASK_HEALTH_ID,
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