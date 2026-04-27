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
    { .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_telecommand_handlers[] = {
    { .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_eps_handlers[] = {
    { .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    { .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_obdh_handlers[] = {
    { .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_adcs_handlers[] = {
    { .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};

static const EVT_StateHandlers_t evt_health_handlers[] = {
    { .on_commissioning = NULL, .on_nominal = NULL, .on_sun_safe = NULL, .on_contingency = NULL },
    // ...
};


const EVT_Config_t evt_cfg_lut[NUM_TASKS] = {

    [TASK_PAYLOAD_ID] = {
        .mask     = EVT_MASK_PAYLOAD,
        .offset   = EVT_OFFSET_PAYLOAD,
        .handlers = evt_payload_handlers,
        .size     = sizeof(evt_payload_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [TASK_COMMS_ID] = {
        .mask     = EVT_MASK_TC,
        .offset   = EVT_OFFSET_TC,
        .handlers = evt_telecommand_handlers,
        .size     = sizeof(evt_telecommand_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [TASK_EPS_ID] = {
        .mask     = EVT_MASK_EPS,
        .offset   = EVT_OFFSET_EPS,
        .handlers = evt_eps_handlers,
        .size     = sizeof(evt_eps_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [TASK_OBDH_ID] = {
        .mask     = EVT_MASK_OBDH,
        .offset   = EVT_OFFSET_OBDH,
        .handlers = evt_obdh_handlers,
        .size     = sizeof(evt_obdh_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [TASK_ADCS_ID] = {
        .mask     = EVT_MASK_ADCS,
        .offset   = EVT_OFFSET_ADCS,
        .handlers = evt_adcs_handlers,
        .size     = sizeof(evt_adcs_handlers) / sizeof(EVT_StateHandlers_t)
    },

    [TASK_HEALTH_ID] = {
        .mask     = EVT_MASK_HEALTH,
        .offset   = EVT_OFFSET_HEALTH,
        .handlers = evt_health_handlers,
        .size     = sizeof(evt_health_handlers) / sizeof(EVT_StateHandlers_t)
    }
};

/* ================= PRIVATE FUNCTION PROTOTYPES ================= */

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
ReturnCode_t EVT_GetHandlers(const EVT_Config_t *cfg, EVT_Index_t index, const EVT_StateHandlers_t **evt_handlers) 
{
    if ((cfg == NULL) || (evt_handlers == NULL) || (index >= cfg->size))
    {
        return NOT_OK;
    }

    /* & is used because we want the direction from the struct that contains the function pointers (handlers) in the handlers table */
    *evt_handlers = &cfg->handlers[index]; 
    
    return OK;
}

ReturnCode_t EVT_GetConfig(OBC_TaskID_t task_id, const EVT_Config_t **cfg)
{
    if (cfg == NULL)
    {
        return NOT_OK;
    }

    if (task_id >= NUM_TASKS)
    {
        return NOT_OK;
    }

    *cfg = &evt_cfg_lut[task_id];

    return OK;
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */

