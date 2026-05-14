#ifndef INC_EVT_CORE_H_
#define INC_EVT_CORE_H_

#include <stdint.h>
#include "common.h"


#define EVT_EVENTS_PER_SLOT 3 // Max number of events that can be encoded in a single notification value, this can be changed if needed
typedef enum {
    /* ================= PAYLOAD EVENTS ================= */
    EVT_PAYLOAD = 0,
    /* ================= TELECOMMAND EVENTS ================= */
    EVT_COMMS = 0,
    EVT_COMMS_1 = 1,
    EVT_COMMS_TO_NOMINAL = 2,
    EVT_COMMS_TO_SUN_SAFE = 3,
    EVT_COMMS_TO_CONTINGENCY = 4,
    EVT_COMMS_TO_COMMISSIONING = 5,
    /* ================= EPS EVENTS ================= */
    EVT_EPS_ = 0,
    EVT_EPS_TO_SUN_SAFE = 1,
    /* ================= OBDH EVENTS ================= */
    EVT_OBDH_ = 0,
    /* ================= ADCS EVENTS ================= */
    EVT_ADCS_ = 0,
    EVT_ADCS_TO_NOMINAL= 1,
    /* ================= HEALTH EVENTS ================= */
    EVT_HEALTH_ = 0,
    EVT_HEALTH_TO_COMMISSIONING = 1,

    EVT_UNDEF = 0x99
} EVT_ID_t;

typedef enum {
    EVT_TYPE_PAYLOAD = 0,
    EVT_TYPE_COMMS,
    EVT_TYPE_EPS,
    EVT_TYPE_OBDH,
    EVT_TYPE_ADCS,
    EVT_TYPE_HEALTH,

    EVT_TYPE_UNDEF = 0x99,
    EVT_NUM_TYPES
} EVT_Type_t;

typedef struct {
    EVT_Type_t type;
    EVT_ID_t   id[EVT_EVENTS_PER_SLOT]; 
} EVT_Decoded_t;

ReturnCode_t evt_decode(TaskNotifyValue_t val, EVT_Decoded_t *decoded_events, uint32_t *num_events);
ReturnCode_t evt_encode(EVT_Type_t type, uint32_t *val, uint32_t num_events, uint32_t *encoded_val);

#endif