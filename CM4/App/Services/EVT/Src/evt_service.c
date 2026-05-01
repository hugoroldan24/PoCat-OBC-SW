/* ================= INCLUDES ================= */
#include "evt_internal.h"
#include "evt.h"
#include "common.h"

/* ================= MACROS AND CONSTANTS ================= */
#define EVT_TABLE_SIZE ARRAY_SIZE(evt_decode_table)
/* ============================================================
 * EVENT WORD BIT LAYOUT (uint32_t)
 * ============================================================
 *
 * This configuration defines how a single event is encoded
 * inside a 32-bit unsigned integer.
 *
 * The event word is divided into fixed-size bitfields:
 *
 *  [ ERROR ][ TELECOMMAND ][ CONDITION ][ SYSTEM ][ TYPE ]
 *    8 bits       8 bits        7 bits      7 bits    2 bits
 *
 * TOTAL: 32 bits (fully utilized)
 *
 * TYPE (2 bits) encodes the high-level event category.
 * The remaining fields encode event identifiers within each
 * category.
 *
 * IMPORTANT:
 * Each field defines the NUMBER OF BITS allocated, meaning:
 *
 *      capacity = 2^(EVT_BITS_x) values
 *
 */

/* ============================================================
 * BIT FIELD SIZES
 * ============================================================
 * Number of bits allocated per category.
 * Defines encoding capacity per field.
 */

#define EVT_BITS_PAYLOAD (4u)

#define EVT_BITS_COMMS   (9u)

#define EVT_BITS_EPS     (4u)

#define EVT_BITS_OBDH    (5u)

#define EVT_BITS_ADCS    (4u)

#define EVT_BITS_HEALTH  (6u)

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

/* ERROR field starts at bit 0 (LSB) */
#define EVT_OFFSET_PAYLOAD  (0u)

/* TELECOMMAND field follows ERROR */
#define EVT_OFFSET_COMMS   (EVT_OFFSET_PAYLOAD + EVT_BITS_PAYLOAD)

/* CONDITION field follows TELECOMMAND */
#define EVT_OFFSET_EPS     (EVT_OFFSET_TC      + EVT_BITS_COMMS)

/* SYSTEM field follows CONDITION */
#define EVT_OFFSET_OBDH    (EVT_OFFSET_EPS     + EVT_BITS_EPS)

/* TYPE field occupies the most significant defined bits */
#define EVT_OFFSET_ADCS    (EVT_OFFSET_OBDH    + EVT_BITS_OBDH)

#define EVT_OFFSET_HEALTH  (EVT_OFFSET_ADCS    + EVT_BITS_ADCS)


/* ============================================================
 * BIT MASKS
 * ============================================================
 *
 * Masks are used to isolate or extract fields from the event word.
 *
 * General formula:
 *   MASK = ((1u << bits) - 1u) << offset
 */

#define EVT_MASK_PAYLOAD  (((uint32_t)((1u << EVT_BITS_PAYLOAD) - 1u)) << EVT_OFFSET_PAYLOAD)

#define EVT_MASK_COMMS    (((uint32_t)((1u << EVT_BITS_TC) - 1u))      << EVT_OFFSET_TC)

#define EVT_MASK_EPS      (((uint32_t)((1u << EVT_BITS_EPS) - 1u))     << EVT_OFFSET_EPS)

#define EVT_MASK_OBDH     (((uint32_t)((1u << EVT_BITS_OBDH) - 1u))   << EVT_OFFSET_OBDH)

#define EVT_MASK_ADCS     (((uint32_t)((1u << EVT_BITS_ADCS) - 1u))    << EVT_OFFSET_ADCS)

#define EVT_MASK_HEALTH   (((uint32_t)((1u << EVT_BITS_HEALTH) - 1u))  << EVT_OFFSET_HEALTH)


/* Generic field extraction macro */
#define EVT_GET_FIELD(val, mask, offset) (((val) & (mask)) >> (offset))

#define EVT_SET_FIELD(val, mask, offset) (((val) << (offset)) & (mask))

/* ================= TYPE DEFINITIONS ================= */

typedef uint32_t EVT_Mask_t;
typedef uint32_t EVT_Offset_t; 

/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */

//El orden importa porque es el usado para decodificar por orden de prioridad!
static const EVT_Config_t evt_cfg_lut[NUM_TASKS] = {

    [EVT_TYPE_EPS] = {
        .mask     = EVT_MASK_EPS,
        .offset   = EVT_OFFSET_EPS,
    },

    [EVT_TYPE_HEALTH] = {
        .mask     = EVT_MASK_HEALTH,
        .offset   = EVT_OFFSET_HEALTH,
    },

    [EVT_TYPE_ADCS] = {
        .mask     = EVT_MASK_ADCS,
        .offset   = EVT_OFFSET_ADCS,
    },

    [EVT_TYPE_OBDH] = {
        .mask     = EVT_MASK_OBDH,
        .offset   = EVT_OFFSET_OBDH,
    },

    [EVT_TYPE_PAYLOAD] = {
        .mask     = EVT_MASK_PAYLOAD,
        .offset   = EVT_OFFSET_PAYLOAD,
    },

    [EVT_TYPE_COMMS] = {
        .mask     = EVT_MASK_COMMS,
        .offset   = EVT_OFFSET_COMMS,
    },
   
};

/* ================= PRIVATE FUNCTION PROTOTYPES ================= */

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
ReturnCode_t evt_encode(EVT_Type_t type, uint32_t val, uint32_t *encoded_val)
{
   if((type >= EVT_NUM_TYPES) || (encoded_val == NULL))
   {
      return NOT_OK;
   }

   const EVT_Config_t *cfg = &evt_cfg_lut[type];
   
   *encoded_val = EVT_SET_FIELD(val, cfg->mask, cfg->offset);

   return OK;
}

ReturnCode_t evt_decode(uin32_t val, EVT_Decoded_t *decoded_events, uint32_t *num_events)
{
   if ((decoded_events == NULL) || (len == 0u) || (num_events == NULL))
   {
      return NOT_OK;
   }

   uint32_t decoded_idx = 0u;
   
   for (uint32_t j = 0u; j < EVT_TABLE_SIZE; j++)
   {
       uint32_t evt_id = EVT_GET_FIELD(val, evt_cfg_lut[j].mask, evt_cfg_lut[j].offset);

       if (evt_id != 0u) /* event was found on the corresponding slot */
       {
           if(i >= EVT_MAX_FIELDS)
           {
              return NOT_OK;
           }
           decoded_events[decoded_idx].id   = evt_id;
           decoded_events[decoded_idx].type = (EVT_Type_t) j;
           i++;
       }          
   }
   *num_events = decoded_idx;
   return OK;
}





/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
