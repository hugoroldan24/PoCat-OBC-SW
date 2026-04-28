/* ================= INCLUDES ================= */
#include "obc_manager.h"
#include "evt_tables.h"
#include "evt_handlers.h"
#include "evt_defs.h"
#include <stddef.h>

/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */

/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */

static const EVT_StateHandlers_t evt_payload_handlers[] = {
    { .id = EVT_PAYLOAD, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .id = EVT_UNDEF,        .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_telecommand_handlers[] = {
    { .id = EVT_UNDEF,            .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .id = EVT_UNDEF,            .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .id = EVT_TC_TESTING,  .on_commissioning = NULL, .on_nominal = testing_handler, .on_sun_safe = NULL, .on_contingency = NULL }
    // ...
};

static const EVT_StateHandlers_t evt_eps_handlers[] = {
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_obdh_handlers[] = {
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_adcs_handlers[] = {
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_health_handlers[] = {
    { .id = EVT_UNDEF, .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

const EVT_Config_t evt_cfg_lut[NUM_TASKS] = {

    [EVT_TYPE_PAYLOAD] = {
        .mask     = EVT_MASK_PAYLOAD,
        .offset   = EVT_OFFSET_PAYLOAD,
        .handlers = evt_payload_handlers,
        .size     = sizeof(evt_payload_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [EVT_TYPE_TC] = {
        .mask     = EVT_MASK_TC,
        .offset   = EVT_OFFSET_TC,
        .handlers = evt_telecommand_handlers,
        .size     = sizeof(evt_telecommand_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [EVT_TYPE_EPS] = {
        .mask     = EVT_MASK_EPS,
        .offset   = EVT_OFFSET_EPS,
        .handlers = evt_eps_handlers,
        .size     = sizeof(evt_eps_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [EVT_TYPE_OBDH] = {
        .mask     = EVT_MASK_OBDH,
        .offset   = EVT_OFFSET_OBDH,
        .handlers = evt_obdh_handlers,
        .size     = sizeof(evt_obdh_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [EVT_TYPE_ADCS] = {
        .mask     = EVT_MASK_ADCS,
        .offset   = EVT_OFFSET_ADCS,
        .handlers = evt_adcs_handlers,
        .size     = sizeof(evt_adcs_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [EVT_TYPE_HEALTH] = {
        .mask     = EVT_MASK_HEALTH,
        .offset   = EVT_OFFSET_HEALTH,
        .handlers = evt_health_handlers,
        .size     = sizeof(evt_health_handlers) / sizeof(EVT_StateHandlers_t)
    }
};

/* ================= PRIVATE FUNCTION PROTOTYPES ================= */

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
const EVT_StateHandlers_t *EVT_GetHandlers(const EVT_Config_t *cfg, EVT_Index_t index) 
{
    if ((cfg == NULL) || (index >= cfg->size))
    {
        return NULL;
    }

    /* & is used because we want the direction from the struct that contains the function pointers (handlers) in the handlers table */    
    return (&cfg->handlers[index]);
}

const EVT_Config_t *EVT_GetConfig(EVT_Type_t type)
{
    if (type >= NUM_TYPES)
    {
        return NULL;
    }

    return &evt_cfg_lut[type];
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */

