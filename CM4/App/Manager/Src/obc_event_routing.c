/* ================= INCLUDES ================= */
#include "obc_manager.h"
#include "obc_handlers.h"
#include "evt_service.h"
#include "utils.h"
/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */

static const OBC_StateHandlers_t obc_payload_handlers[] = {
    { .id = EVT_PAYLOAD, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .id = EVT_UNDEF,        .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const OBC_StateHandlers_t obc_comms_handlers[] = {
    { .id = EVT_UNDEF,            .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .id = EVT_UNDEF,            .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .id = EVT_COMMS_TESTING,  .on_commissioning = NULL, .on_nominal = testing_handler, .on_sun_safe = NULL, .on_contingency = NULL }
    // ...
};

static const OBC_StateHandlers_t obc_eps_handlers[] = {
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const OBC_StateHandlers_t obc_obdh_handlers[] = {
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const OBC_StateHandlers_t obc_evt_adcs_handlers[] = {
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const OBC_StateHandlers_t obc_health_handlers[] = {
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
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
        .table = evt_obdh_handlers,
        .handlers_count = ARRAY_SIZE(evt_obdh_handlers)
    },

    [EVT_TYPE_ADCS] = {
        .table = obc_evt_adcs_handlers,
        .handlers_count = ARRAY_SIZE(obc_evt_adcs_handlers)
    },

    [EVT_TYPE_HEALTH] = {
        .table = obc_health_handlers,
        .handlers_count = ARRAY_SIZE(obc_health_handlers)
    }
};


static const EVT_Type_t obc_to_evt_map[OBC_NUM_TASKS] = {
    [OBC_TASK_PAYLOAD_ID] = EVT_TYPE_PAYLOAD,
    [OBC_TASK_COMMS_ID]   = EVT_TYPE_COMMS,
    [OBC_TASK_EPS_ID]     = EVT_TYPE_EPS,
    [OBC_TASK_OBDH_ID]    = EVT_TYPE_OBDH,
    [OBC_TASK_ADCS_ID]    = EVT_TYPE_ADCS,
    [OBC_TASK_HEALTH_ID]  = EVT_TYPE_HEALTH
};

/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
const OBC_StateHandlers_t *obc_resolve_event_handler(EVT_Decoded_t *decoded_event) 
{
    if ((decoded_event == NULL))
    {
        return NULL;
    }

    EVT_ID_t   evt_id   = decoded_event->id;
    EVT_Type_t evt_type = decoded_event->type;
    
    
    if ((evt_type >= EVT_NUM_TYPES))
    {
        return NULL:
    }

    if((evt_id >= obc_event_tables[evt_type].handlers_count))
    {
        return NULL;
    }

    /* & is used because we want the direction from the struct that contains the function pointers (handlers) in the handlers table */    
    return (&obc_event_tables[evt_type][evt_id]);
}
/* ================= PRIVATE FUNCTION DEFINITIONS ================= */