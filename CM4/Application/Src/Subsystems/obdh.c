// oversees the management of internal data within the spacecraft. Its primary focus includes 
// housekeeping data, scientific data, and configurations, as well as managing access to flash 
// memory. (primary focus now is saving and retrieving data from flash)

/* ================= INCLUDES ================= */
#include "obdh.h"
#include "common.h"
#include <stdio.h>

/* ================= MACROS AND CONSTANTS ================= */
/* ================= TYPE DEFINITIONS ================= */
/* ================= GLOBAL VARIABLES ================= */
/* ================= MODULE-LEVEL VARIABLES ================= */
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void setup_obdh(void);
static void process_obdh(void);

/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void obdh_task(void *pv_parameters) {

    setup_obdh();

    for (;;) {
        process_obdh();
    }

}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void setup_obdh(void) {

    printf("Setting up OBDH...\n");
    // Apply the default configuration

}

static void process_obdh(void) {

    TaskNotifyValue_t value;
    waitForNotification(&value);
    printf("Processing OBDH...\n");

    // Gestión de la flash:
    // Leemos datos de la cola de la tarea (donde habran peticiones de read o write de otras tareas
    // que quieran acceder a la memoria flash)
    // La información de cada elemento en la cola será el siguiente struct:
    //  *   - op      : FLASH_READ o FLASH_WRITE
    //  *   - addr    : dirección en flash
    //  *   - len     : número de bytes
    //  *   - buf     : puntero al buffer (src en WRITE, dst en READ)
    //  *   - client  : TaskHandle_t de la tarea solicitante (para notificación de fin)
    // A partir de esto si es FLASH_READ leemos la flash y ponemos la info en buf, si es FLASH_WRITE 
    // escribimos la info de buf en la flash
    // Si es FLASH_READ notificamos a la tarea solicitante que la información esta disponible en la 
    // posición que nos ha pasado con buf (aquí suponemos que la tarea solicitante sabe cuanto ocupa la
    // información que pide de flash).

}
