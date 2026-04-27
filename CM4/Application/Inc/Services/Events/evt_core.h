#ifndef INC_EVT_CORE_H_
#define INC_EVT_CORE_H_


#include "evt_defs.h"
#include "obc_manager.h"
#include "common.h"

ReturnCode_t EVT_ResolveHandlersFromNotify(const EVT_StateHandlers_t **handlers, TaskNotifyValue_t val);

#endif