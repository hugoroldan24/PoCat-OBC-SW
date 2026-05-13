#ifndef INC_OBC_EVENT_MAPPING_H
#define INC_OBC_EVENT_MAPPING_H

#include <stdint.h>
#include "obc_manager.h"

const OBC_Handler_t *obc_resolve_event_handler(EVT_ID_t evt_id, EVT_Type_t evt_type); 

#endif