#ifndef INC_OBC_MANAGER_H
#define INC_OBC_MANAGER_H


#include "main.h"
#include <stdint.h>
#include "evt_service.h"
#include "logging.h"

#define COMMS_PERIOD_MS    2500 // ??
#define ADCS_PERIOD_MS     2000 // ??
#define HEALTH_PERIOD_MS   3500 // ??
#define EPS_PERIOD_MS      4000 // ??

extern TaskHandle_t obc_task_handle;
extern TaskHandle_t payload_task_handle;
extern TaskHandle_t eps_task_handle;
extern TaskHandle_t comms_task_handle;
extern TaskHandle_t obdh_task_handle;
extern TaskHandle_t adcs_task_handle;
extern TaskHandle_t health_task_handle;

/* Satellite modes */
typedef enum {
    OBC_STATE_NOMINAL = 0u,
    OBC_STATE_CONTINGENCY,
    OBC_STATE_SUN_SAFE,
    OBC_STATE_COMMISSIONING
} OBC_SatelliteState_t;


typedef enum {
   TASK_PAYLOAD_ID = 0,
   TASK_COMMS_ID,
   TASK_EPS_ID,
   TASK_OBDH_ID,
   TASK_ADCS_ID,
   TASK_HEALTH_ID,
   OBC_NUM_TASKS
} OBC_TaskID_t;

typedef struct {
    EVT_ID_t id;
    ReturnCode_t (*action)(OBC_SatelliteState_t *state);
} OBC_Handler_t;

ReturnCode_t obc_init_task();
ReturnCode_t obc_send_event_from_task(OBC_TaskID_t id, EVT_ID_t *val, uint32_t len);


#endif