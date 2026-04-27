#ifndef INC_EVT_TABLES_H_
#define INC_EVT_TABLES_H_

#include "obc_manager.h"
#include "evt_defs.h"
#include "common.h"


ReturnCode_t EVT_GetHandlers(const EVT_Config_t* cfg, uint32_t index, const EVT_StateHandlers_t **handlers);
ReturnCode_t EVT_GetConfig(OBC_TaskID_t task_id, const EVT_Config_t **cfg);

#endif