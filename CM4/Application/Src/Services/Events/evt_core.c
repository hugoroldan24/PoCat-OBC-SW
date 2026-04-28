/* ================= INCLUDES ================= */
#include "evt_core.h"
#include "evt_tables.h"
#include <stddef.h>

/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */


/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
/*
 * EVT_ResolveHandlersFromNotify
 * --------------------
 * Takes the current notification snapshots from all channels and maps the active bits
 * to their corresponding OBC events using a predefined event table.
 *
 * Parameters:
 *   snap   - The NotificationSnapshot_t struct containing snapshots of all notification channels.
 *   events - Pointer to an array of EVT_StateHandlers_t where detected events will be stored.
 *
 * Returns:
 *   The number of events detected in the snapshots and mapped to the events array.
 *
 * Notes:
 *   Only bits set to 1 in the snapshots are considered; each active bit corresponds to one event.
 *   The function allows the OBC Task Manager to build a list of events to process in order.
 */
ReturnCode_t EVT_ResolveHandlersFromNotify(const EVT_StateHandlers_t **handlers, TaskNotifyValue_t val)
{
    if (handlers == NULL)
    {
        return NOT_OK;
    }

    EVT_Type_t type = EVT_GET_FIELD(val, EVT_MASK_TYPE, EVT_OFFSET_TYPE);

    if (type >= NUM_TYPES)
    {
        return NOT_OK;
    }

    const EVT_Config_t *evt_config = EVT_GetConfig(type);

    if(evt_config == NULL)
    {
        return NOT_OK;
    }

    EVT_Index_t handlerIndex = EVT_GET_FIELD(val, evt_config->mask, evt_config->offset);

    /* handlerIndex verification is performed in EVT_GetHandlers */

    *handlers = EVT_GetHandlers(evt_config, handlerIndex);

    if(*handlers == NULL)
    {
        return NOT_OK;
    }

    return OK;
}



/* ================= PRIVATE FUNCTION DEFINITIONS ================= */