/* ================= INCLUDES ================= */
#include "obc_manager.h"
#include "obc_handlers.h"
#include "evt_service.h"
#include "utils.h"
#include <stdint.h>

/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
typedef struct{
    const OBC_Handler_t *table;
    uint32_t handlers_count;
}OBC_HandlerTableEntry_t;

/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */

static const OBC_Handler_t obc_payload_handlers[] = {
    { .id = EVT_PAYLOAD, .handler = NULL },
    { .id = EVT_UNDEF,   .handler = NULL },
    // ...
};

static const OBC_Handler_t obc_comms_handlers[] = {
    { .id = EVT_UNDEF,                  .handler = NULL},
    { .id = EVT_UNDEF,                  .handler = NULL},
    { .id = EVT_COMMS_TO_NOMINAL,       .handler = switch_state_to_nominal},
    { .id = EVT_COMMS_TO_SUN_SAFE,      .handler = switch_state_to_sun_safe},
    { .id = EVT_COMMS_TO_CONTINGENCY,   .handler = switch_state_to_contingency},
    { .id = EVT_COMMS_TO_COMMISSIONING, .handler = switch_state_to_commissioning},
    // ...
};

static const OBC_Handler_t obc_eps_handlers[] = {
    { .id = EVT_UNDEF,           .handler = NULL},
    { .id = EVT_EPS_TO_SUN_SAFE, .handler = switch_state_to_sun_safe},
    // ...
};

static const OBC_Handler_t obc_obdh_handlers[] = {
    { .id = EVT_UNDEF, .handler = NULL},
    // ...
};

static const OBC_Handler_t obc_adcs_handlers[] = {
    { .id = EVT_UNDEF,           .handler = NULL},
    { .id = EVT_ADCS_TO_NOMINAL, .handler = switch_state_to_nominal},
    // ...
};

static const OBC_Handler_t obc_health_handlers[] = {
    { .id = EVT_UNDEF,                   .handler = NULL},
    { .id = EVT_HEALTH_TO_COMMISSIONING, .handler = switch_state_to_commissioning},
     // ...
    // ...
};

static const OBC_HandlerTableEntry_t obc_event_tables[] = {
    [EVT_TYPE_PAYLOAD] = {
        .table = obc_payload_handlers,
        .handlers_count = ARRAY_SIZE(obc_payload_handlers)
    },

    [EVT_TYPE_COMMS] = {
        .table = obc_comms_handlers,
        .handlers_count = ARRAY_SIZE(obc_comms_handlers)
    },

    [EVT_TYPE_EPS] = {
        .table = obc_eps_handlers,
        .handlers_count = ARRAY_SIZE(obc_eps_handlers)
    },

    [EVT_TYPE_OBDH] = {
        .table = obc_obdh_handlers,
        .handlers_count = ARRAY_SIZE(obc_obdh_handlers)
    },

    [EVT_TYPE_ADCS] = {
        .table = obc_adcs_handlers,
        .handlers_count = ARRAY_SIZE(obc_adcs_handlers)
    },

    [EVT_TYPE_HEALTH] = {
        .table = obc_health_handlers,
        .handlers_count = ARRAY_SIZE(obc_health_handlers)
    }
};


static const EVT_Type_t obc_to_evt_map[OBC_NUM_TASKS] = {
    [TASK_PAYLOAD_ID] = EVT_TYPE_PAYLOAD,
    [TASK_COMMS_ID]   = EVT_TYPE_COMMS,
    [TASK_EPS_ID]     = EVT_TYPE_EPS,
    [TASK_OBDH_ID]    = EVT_TYPE_OBDH,
    [TASK_ADCS_ID]    = EVT_TYPE_ADCS,
    [TASK_HEALTH_ID]  = EVT_TYPE_HEALTH
};

/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
const OBC_Handler_t *obc_resolve_event_handler(EVT_ID_t evt_id, EVT_Type_t evt_type) 
{
  
    if ((evt_type >= EVT_NUM_TYPES))
    {
        return NULL;
    }

    if((evt_id >= obc_event_tables[evt_type].handlers_count))
    {
        return NULL;
    }

    /* & is used because we want the direction from the struct that contains the function pointers (handlers) in the handlers table */    
    return (&obc_event_tables[evt_type][evt_id]);
}
/* ================= PRIVATE FUNCTION DEFINITIONS ================= */