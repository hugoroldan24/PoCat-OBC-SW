#ifndef INC_EVT_TABLES_H_
#define INC_EVT_TABLES_H_

#include "obc_manager.h"
#include "evt_defs.h"
#include "common.h"


const EVT_StateHandlers_t *EVT_GetHandlers(const EVT_Config_t *cfg, EVT_Index_t index);
const EVT_Config_t *EVT_GetConfig(EVT_Type_t task_id);

#endif