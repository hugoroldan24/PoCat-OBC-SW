#ifndef INC_EVT_CORE_H_
#define INC_EVT_CORE_H_

#include <stdint.h>
#include "common.h"

#define EVT_MAX_FIELDS (6u)

typedef struct {
    EVT_Mask_t   mask;
    EVT_Offset_t offset;
    EVT_Type_t   type;
} EVT_Config_t;

typedef enum {
    /* ================= PAYLOAD EVENTS ================= */
    EVT_PAYLOAD = 1,
    /* ================= TELECOMMAND EVENTS ================= */
    EVT_COMMS = 1,
    EVT_COMMS_1 = 2,
    EVT_COMMS_TESTING = 3,
    /* ================= EPS EVENTS ================= */
    EVT_EPS = 1,
    /* ================= OBDH EVENTS ================= */
    EVT_OBDH_ = 1,
    /* ================= ADCS EVENTS ================= */
    EVT_ADCS_ = 1,
    /* ================= HEALTH EVENTS ================= */
    EVT_HEALTH_ = 1,

    EVT_UNDEF = 0x99
} EVT_ID_t;

typedef enum {
    EVT_TYPE_PAYLOAD = 0,
    EVT_TYPE_COMMS,
    EVT_TYPE_EPS,
    EVT_TYPE_OBDH,
    EVT_TYPE_ADCS,
    EVT_TYPE_HEALTH,

    EVT_NUM_TYPES
} EVT_Type_t;

typedef struct {
    EVT_Type_t type;
    EVT_ID_t   id;
} EVT_Decoded_t;

ReturnCode_t evt_decode(TaskNotifyValue_t val, EVT_Decoded_t *decoded_events, uint32_t *num_events);
ReturnCode_t evt_encode(EVT_Type_t type, uint32_t val, uint32_t *encoded_val);

#endif