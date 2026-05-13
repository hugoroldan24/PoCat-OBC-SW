/* ================= INCLUDES ================= */
#include "obc_manager.h"

/* ================= MACROS AND CONSTANTS ================= */
#define LED_COMMISSIONING_PORT GPIOI
#define LED_CONTENGENCY_PORT GPIOF
#define LED_SUN_SAFE_PORT GPIOG
#define LED_NOMINAL_PORT GPIOG

#define LED_COMMISSIONING GPIO_PIN_5 /*I*/
#define LED_CONTINGENCY   GPIO_PIN_1 /*F*/
#define LED_SUN_SAFE     GPIO_PIN_15 /*G*/
#define LED_NOMINAL      GPIO_PIN_8 /*G*/

/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void switch_state_to_nominal(OBC_SatelliteState_t *state)
{
    if(OBC_STATE_NOMINAL == *state)
    {
        return;
    }
    else
    {
        HAL_GPIO_WritePin(LED_COMMISSIONING_PORT, LED_COMMISSIONING, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_CONTENGENCY_PORT, LED_CONTINGENCY, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_NOMINAL_PORT, LED_NOMINAL, GPIO_PIN_SET);
    }
    *state = OBC_STATE_NOMINAL;
    return;
}

void switch_state_to_commissioning(OBC_SatelliteState_t *state)
{
    if(OBC_STATE_COMMISSIONING == *state)
    {
       return;
    }
    else
    {
       HAL_GPIO_WritePin(LED_NOMINAL_PORT, LED_NOMINAL, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_CONTENGENCY_PORT, LED_CONTINGENCY, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_COMMISSIONING_PORT, LED_COMMISSIONING, GPIO_PIN_SET);     
    }
   *state = OBC_STATE_COMMISSIONING;
}

void switch_state_to_sun_safe(OBC_SatelliteState_t *state)
{
   if(OBC_STATE_SUN_SAFE == *state)
   {
      return;
   }
   else
   {
       HAL_GPIO_WritePin(LED_NOMINAL_PORT, LED_NOMINAL, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_COMMISSIONING_PORT, LED_COMMISSIONING, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_CONTENGENCY_PORT, LED_CONTINGENCY, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_SET);
   }
   *state = OBC_STATE_SUN_SAFE;
}

void switch_state_to_contingency(OBC_SatelliteState_t *state)
{
   if(OBC_STATE_SUN_SAFE == *state)
   {
      return;
   }
   else
   {
       HAL_GPIO_WritePin(LED_NOMINAL_PORT, LED_NOMINAL, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_COMMISSIONING_PORT, LED_COMMISSIONING, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_RESET);
       HAL_GPIO_WritePin(LED_CONTENGENCY_PORT, LED_CONTINGENCY, GPIO_PIN_SET);
   }
   *state = OBC_STATE_CONTINGENCY;  
}
/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
