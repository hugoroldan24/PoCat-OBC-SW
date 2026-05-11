#ifndef INC_OBC_EVENT_MAPPING_H
#define INC_OBC_EVENT_MAPPING_H

#include <stdint.h>
#include "obc_manager.h"

const EVT_StateHandlers_t* obc_resolve_event_handler(EVT_Decoded_t *decoded_event);

#endif