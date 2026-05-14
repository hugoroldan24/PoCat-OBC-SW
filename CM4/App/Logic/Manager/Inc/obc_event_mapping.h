#ifndef INC_OBC_EVENT_MAPPING_H
#define INC_OBC_EVENT_MAPPING_H

#include <stdint.h>
#include "obc_manager.h"

EVT_Type_t obc_get_evt_type(OBC_TaskID_t id);
const OBC_Handler_t *obc_resolve_event_handler(EVT_ID_t evt_id, EVT_Type_t evt_type); 

#endif