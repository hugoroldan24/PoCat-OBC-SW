#ifndef INC_COMMON_HANDLERS_H_
#define INC_COMMON_HANDLERS_H_

#include "obc_manager.h"

void switch_state_to_nominal(OBC_SatelliteState_t *state);
void switch_state_to_commissioning(OBC_SatelliteState_t *state);
void switch_state_to_sun_safe(OBC_SatelliteState_t *state);
void switch_state_to_contingency(OBC_SatelliteState_t *state);

#endif