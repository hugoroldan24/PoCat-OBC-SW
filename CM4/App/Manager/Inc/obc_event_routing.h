#ifndef INC_OBC_EVENT_ROUTING_H
#define INC_OBC_EVENT_ROUTING_H

#include <stdint.h>
#include "obc_manager.h"

typedef struct{
    OBC_StateHandlers_t table;
    uint32_t handlers_count;
} OBC_HandlerTableEntry_t;

const EVT_StateHandlers_t* obc_resolve_event_handler(EVT_Decoded_t *decoded_event);

#endif