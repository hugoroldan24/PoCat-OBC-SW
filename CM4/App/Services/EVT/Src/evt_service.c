/* ================= INCLUDES ================= */
#include <stddef.h>
#include "common.h"
#include "evt_service.h"
/* ================= MACROS AND CONSTANTS ================= */
#define EVT_NUM_TYPES (6u)

#define EVT_TABLE_SIZE ARRAY_SIZE(evt_decode_table)

#define EVT_BITS_DATA       (9u)

#define EVT_BITS_NUM_EVENTS (2u)

#define EVT_BITS_TYPE       (3u)


/* ============================================================
 * BIT OFFSETS
 * ============================================================
 *
 * Defines the starting bit position of each field inside
 * the 32-bit event word.
 *
 * Offsets are computed cumulatively to guarantee that
 * fields do not overlap.
 */

#define EVT_OFFSET_DATA       (0u)

#define EVT_OFFSET_TYPE       (EVT_OFFSET_DATA + EVT_BITS_DATA * 3u) /* 3 simultaneous events */

#define EVT_OFFSET_NUM_EVENTS (EVT_OFFSET_TYPE    + EVT_BITS_TYPE)


/* ============================================================
 * BIT MASKS
 * ============================================================
 *
 * Masks are used to isolate or extract fields from the event word.
 *
 * General formula:
 *   MASK = ((1u << bits) - 1u) << offset
 */

#define EVT_MASK_DATA       (((uint32_t)((1u << EVT_BITS_DATA) - 1u)) << EVT_OFFSET_DATA)
 
#define EVT_MASK_TYPE       (((uint32_t)((1u << EVT_BITS_TYPE) - 1u))   << EVT_OFFSET_TYPE)

#define EVT_MASK_NUM_EVENTS (((uint32_t)((1u << EVT_BITS_NUM_EVENTS) - 1u))  << EVT_OFFSET_NUM_EVENTS)



/* ================= TYPE DEFINITIONS ================= */


/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */



/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static inline uint32_t evt_set_field(uint32_t field_val, uint32_t mask, uint32_t offset);
static inline uint32_t evt_get_field(uint32_t val, uint32_t mask, uint32_t offset);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
ReturnCode_t evt_encode(EVT_Type_t type, uint32_t *val, uint32_t num_events, uint32_t *encoded_val)
{
   if((type >= EVT_NUM_TYPES) || (encoded_val == NULL) || (num_events > EVT_EVENTS_PER_SLOT))
   {
      return NOT_OK;
   }

   uint32_t l_encoded_val = 0u;
   uint32_t l_offset = 0u;
   uint32_t l_mask = 0u;

   for(uint32_t i = 0u; i < num_events; i++)
   {    
      l_mask   = (l_mask << (EVT_BITS_DATA * i));
      l_offset = (l_offset + (EVT_BITS_DATA * i));
      l_encoded_val |= evt_set_field(val[i], l_mask, l_offset);           
   }
      
   l_encoded_val |= evt_set_field(num_events, EVT_MASK_NUM_EVENTS, EVT_OFFSET_NUM_EVENTS) | 
                    evt_set_field(type, EVT_MASK_TYPE, EVT_OFFSET_TYPE);

   *encoded_val = l_encoded_val;

   return OK;
}

/* Devuelve un struct que contiene el tipo del subsistema y un array con los eventos*/
ReturnCode_t evt_decode(uint32_t val, EVT_Decoded_t *decoded_events, uint32_t *num_events)
{
   if ((decoded_events == NULL)  || (num_events == NULL))
   {
      return NOT_OK;
   }

   uint32_t l_offset = 0u;
   uint32_t l_mask = 0u;

   EVT_Type_t type = (EVT_Type_t)evt_get_field(val, EVT_MASK_TYPE, EVT_OFFSET_TYPE);
   uint32_t   l_num_events = (int32_t)evt_get_field(val, EVT_MASK_NUM_EVENTS, EVT_OFFSET_NUM_EVENTS);

   decoded_events->type = type;

   for(uint32_t i = 0u; i < l_num_events; i++)
   {
       l_mask   = (l_mask   << (EVT_BITS_DATA * i));
       l_offset = (l_offset +  (EVT_BITS_DATA * i));
       decoded_events->id[i] = (EVT_ID_t)evt_get_field(val, l_mask, l_offset);   
   }
   
   *num_events = l_num_events;
   return OK;
}





/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static inline uint32_t evt_set_field(uint32_t field_val, uint32_t mask, uint32_t offset)
{
    return (field_val << offset) & mask;
}

static inline uint32_t evt_get_field(uint32_t val, uint32_t mask, uint32_t offset)
{
    return (val & mask) >> offset;
}