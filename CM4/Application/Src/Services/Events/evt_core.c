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
    ReturnCode_t retVal = NOT_OK;
    const EVT_Config_t *evt_config;


    if (handlers == NULL)
    {
        return retVal;
    }

    OBC_TaskID_t task_id = EVT_GET_FIELD(val, EVT_MASK_TASK_ID, EVT_OFFSET_TASK_ID);

    if (task_id >= NUM_TASKS)
    {
        return retVal;
    }

    retVal = EVT_GetConfig(task_id, &evt_config);

    if(retVal == OK)
    {
        return retVal;
    }

    EVT_Index_t handlerIndex = EVT_GET_FIELD(val, evt_config->mask, evt_config->offset);

    retVal = EVT_GetHandlers(evt_config, handlerIndex, handlers);
    return retVal;
}



/* ================= PRIVATE FUNCTION DEFINITIONS ================= */