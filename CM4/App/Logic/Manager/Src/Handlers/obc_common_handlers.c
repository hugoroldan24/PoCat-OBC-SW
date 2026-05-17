/* ================= INCLUDES ================= */
#include <stdio.h>
#include "obc_manager.h"
/* ================= MACROS AND CONSTANTS ================= */
#define LED_COMMISSIONING_PORT GPIOF
#define LED_CONTINGENCY_PORT GPIOF
#define LED_SUN_SAFE_PORT GPIOG
#define LED_NOMINAL_PORT GPIOG

#define LED_COMMISSIONING GPIO_PIN_0 /*F*/
#define LED_CONTINGENCY   GPIO_PIN_1 /*F*/
#define LED_SUN_SAFE     GPIO_PIN_15 /*G*/
#define LED_NOMINAL      GPIO_PIN_8 /*G*/

/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
ReturnCode_t switch_state_to_nominal(OBC_SatelliteState_t *state)
{
   if(state == NULL)
   {
       return NOT_OK;
   }
   switch(*state)
   {
      case OBC_STATE_NOMINAL:
         LOGGING("Already in nominal mode!!!\n");
         return NOT_OK;
      case OBC_STATE_SUN_SAFE:
         HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_RESET);
         LOGGING("Switched to NOMINAL mode from sun safe\n");
         break;
      case OBC_STATE_COMMISSIONING:
         HAL_GPIO_WritePin(LED_COMMISSIONING_PORT, LED_COMMISSIONING, GPIO_PIN_RESET);
         LOGGING("Switched to NOMINAL mode from commissioning\n");
         break;
      case OBC_STATE_CONTINGENCY:
         HAL_GPIO_WritePin(LED_CONTINGENCY_PORT, LED_CONTINGENCY, GPIO_PIN_RESET);
         LOGGING("Switched to NOMINAL mode from contingency\n");
         break;
      default:
         LOGGING("Invalid state!!!\n");
         return NOT_OK;
   }

   HAL_GPIO_WritePin(LED_NOMINAL_PORT, LED_NOMINAL, GPIO_PIN_SET);   
   *state = OBC_STATE_NOMINAL;  
   return OK;
}

ReturnCode_t switch_state_to_commissioning(OBC_SatelliteState_t *state)
{
   if(state == NULL)
   {
      return NOT_OK;
   }
  
   switch(*state)
   {
      case OBC_STATE_COMMISSIONING:
         LOGGING("Already in commissioning mode!!!\n");
         return NOT_OK;
      case OBC_STATE_SUN_SAFE:
         HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_RESET);
         LOGGING("Switched to COMMISSIONING mode from sun safe\n");
         break;
      case OBC_STATE_CONTINGENCY:
         HAL_GPIO_WritePin(LED_CONTINGENCY_PORT, LED_CONTINGENCY, GPIO_PIN_RESET);
         LOGGING("Switched to COMMISSIONING mode from contingency\n");
         break;
      case OBC_STATE_NOMINAL:
         HAL_GPIO_WritePin(LED_NOMINAL_PORT, LED_NOMINAL, GPIO_PIN_RESET);
         LOGGING("Switched to COMMISSIONING mode from nominal\n");
         break;
      default:
         LOGGING("Invalid state!!!\n");
         return NOT_OK;
   }

   HAL_GPIO_WritePin(LED_COMMISSIONING_PORT, LED_COMMISSIONING, GPIO_PIN_SET);   
   *state = OBC_STATE_COMMISSIONING;  
   return OK;
}

ReturnCode_t switch_state_to_sun_safe(OBC_SatelliteState_t *state)
{
   if(state == NULL)
   {
      return NOT_OK;
   }
   switch(*state)
   {
      case OBC_STATE_SUN_SAFE:
         LOGGING("Already in sun safe mode!!!\n");
         return NOT_OK;
      case OBC_STATE_COMMISSIONING:
         HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_RESET);
         LOGGING("Switched to SUN SAFE mode from commissioning\n");
         break;
      case OBC_STATE_CONTINGENCY:
         HAL_GPIO_WritePin(LED_CONTINGENCY_PORT, LED_CONTINGENCY, GPIO_PIN_RESET);
         LOGGING("Switched to SUN SAFE mode from contingency\n");
         break;
      case OBC_STATE_NOMINAL:
         HAL_GPIO_WritePin(LED_NOMINAL_PORT, LED_NOMINAL, GPIO_PIN_RESET);
         LOGGING("Switched to SUN SAFE mode from nominal\n");
         break;
      default:
         LOGGING("Invalid state!!!\n");
         return NOT_OK;
   }

   HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_SET);   
   *state = OBC_STATE_SUN_SAFE;  
   return OK;
}


ReturnCode_t switch_state_to_contingency(OBC_SatelliteState_t *state)
{  
   if(state == NULL)
   {
      return NOT_OK;
   }
   switch(*state)
   {
      case OBC_STATE_CONTINGENCY:
         LOGGING("Already in contingency mode!!!\n");
         return NOT_OK;
      case OBC_STATE_SUN_SAFE:
         HAL_GPIO_WritePin(LED_SUN_SAFE_PORT, LED_SUN_SAFE, GPIO_PIN_RESET);
         LOGGING("Switched to CONTINGENCY mode from sun safe\n");
         break;
      case OBC_STATE_COMMISSIONING:
         HAL_GPIO_WritePin(LED_COMMISSIONING_PORT, LED_COMMISSIONING, GPIO_PIN_RESET);
         LOGGING("Switched to CONTINGENCY mode from commissioning\n");
         break;
      case OBC_STATE_NOMINAL:
         HAL_GPIO_WritePin(LED_NOMINAL_PORT, LED_NOMINAL, GPIO_PIN_RESET);
         LOGGING("Switched to CONTINGENCY mode from nominal\n");
         break;
      default:
         LOGGING("Invalid state!!!\n");
         return NOT_OK;
   }

   HAL_GPIO_WritePin(LED_CONTINGENCY_PORT, LED_CONTINGENCY, GPIO_PIN_SET);   
   *state = OBC_STATE_CONTINGENCY;  
   return OK;
}
/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
