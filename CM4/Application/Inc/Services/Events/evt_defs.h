#ifndef INC_EVT_DEFS_H
#define INC_EVT_DEFS_H

#include <stdint.h>


typedef enum {
    /* ================= PAYLOAD EVENTS ================= */
    EVT_PAYLOAD = 0,
    /* ================= TELECOMMAND EVENTS ================= */
    EVT_TC = 0,
    EVT_TC_1 = 1,
    EVT_TC_TESTING = 2,
    /* ================= EPS EVENTS ================= */
    EVT_EPS = 0,
    /* ================= OBDH EVENTS ================= */
    EVT_OBDH_ = 0,
    /* ================= ADCS EVENTS ================= */
    EVT_ADCS_ = 0,
    /* ================= HEALTH EVENTS ================= */
    EVT_HEALTH_ = 0,

    EVT_UNDEF = 0x99

} EVT_ID_t;

typedef struct {
    EVT_ID_t id;
    void (*on_commissioning)(void);
    void (*on_nominal)(void);
    void (*on_sun_safe)(void);
    void (*on_contingency)(void);
} EVT_StateHandlers_t;

typedef enum {
    EVT_TYPE_PAYLOAD = 0,
    EVT_TYPE_TC,
    EVT_TYPE_EPS,
    EVT_TYPE_OBDH,
    EVT_TYPE_ADCS,
    EVT_TYPE_HEALTH,

    NUM_TYPES
} EVT_Type_t;

typedef uint32_t EVT_Mask_t;
typedef uint32_t EVT_Offset_t; 
typedef uint32_t EVT_Index_t;


typedef struct {
    EVT_Mask_t mask;
    EVT_Offset_t offset;
    const EVT_StateHandlers_t *handlers;
    EVT_Index_t size;
} EVT_Config_t;

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

#define EVT_BITS_TC      (8u)

#define EVT_BITS_EPS     (4u)

#define EVT_BITS_OBDH    (4u)

#define EVT_BITS_ADCS    (4u)

#define EVT_BITS_HEALTH  (5u)

#define EVT_BITS_TYPE    (3u)


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
#define EVT_OFFSET_TC      (EVT_OFFSET_PAYLOAD + EVT_BITS_PAYLOAD)

/* CONDITION field follows TELECOMMAND */
#define EVT_OFFSET_EPS     (EVT_OFFSET_TC      + EVT_BITS_TC)

/* SYSTEM field follows CONDITION */
#define EVT_OFFSET_OBDH    (EVT_OFFSET_EPS     + EVT_BITS_EPS)

/* TYPE field occupies the most significant defined bits */
#define EVT_OFFSET_ADCS    (EVT_OFFSET_OBDH    + EVT_BITS_OBDH)

#define EVT_OFFSET_HEALTH  (EVT_OFFSET_ADCS    + EVT_BITS_ADCS)

#define EVT_OFFSET_TYPE    (EVT_OFFSET_HEALTH  + EVT_BITS_HEALTH)


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

#define EVT_MASK_TC       (((uint32_t)((1u << EVT_BITS_TC) - 1u))      << EVT_OFFSET_TC)

#define EVT_MASK_EPS      (((uint32_t)((1u << EVT_BITS_EPS) - 1u))     << EVT_OFFSET_EPS)

#define EVT_MASK_OBDH     (((uint32_t)((1u << EVT_BITS_OBDH) - 1u))   << EVT_OFFSET_OBDH)

#define EVT_MASK_ADCS     (((uint32_t)((1u << EVT_BITS_ADCS) - 1u))    << EVT_OFFSET_ADCS)

#define EVT_MASK_HEALTH   (((uint32_t)((1u << EVT_BITS_HEALTH) - 1u))  << EVT_OFFSET_HEALTH)

#define EVT_MASK_TYPE     (((uint32_t)((1u << EVT_BITS_TYPE) - 1u))    << EVT_OFFSET_TYPE)


/* Generic field extraction macro */
#define EVT_GET_FIELD(val, mask, offset) (((val) & (mask)) >> (offset))

#define EVT_SET_FIELD(val, mask, offset) (((val) << (offset)) & (mask))


#endif