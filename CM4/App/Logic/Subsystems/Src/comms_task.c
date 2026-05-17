/* ================= INCLUDES ================= */
#include "comms_task.h"
#include "evt_service.h"
#include "common.h"
#include "obc_manager.h"

#include <stdio.h>
#include <string.h>

/* ================= MACROS AND CONSTANTS ================= */
#define RECEIVE_COMMAND(buff) UART7_ReceiveLine(buff, sizeof(buff))

/* ================= TYPE DEFINITIONS ================= */

/* ================= GLOBAL VARIABLES ================= */
extern UART_HandleTypeDef huart7;
 
/* ================= MODULE-LEVEL VARIABLES ================= */
static TickType_t xLastWakeTime;
static TickType_t xPeriod;

static char msg_buffer[20];
static uint32_t rx_tc;
/* ================= PRIVATE FUNCTION PROTOTYPES ================= */
static void setup_comms(void);
static void process_comms(void);
static HAL_StatusTypeDef UART7_ReceiveLine(char *buffer, uint16_t max_len);
static void check_telecommand_rx(uint32_t *rx_tc);


/* ================= PUBLIC FUNCTION DEFINITIONS ================= */
void comms_task(void *pv_parameters){
    setup_comms();

    for(;;)
    {
        process_comms();
    }
    /*TODO implementation*/
}

/* ================= PRIVATE FUNCTION DEFINITIONS ================= */
static void setup_comms(void)
{
    printf("Setting up COMMS...\n");
    // Apply the default configuration
    xPeriod = pdMS_TO_TICKS(COMMS_PERIOD_MS);

    xLastWakeTime = xTaskGetTickCount();
}

static void process_comms(void)
{

    check_telecommand_rx(&telecommand);
    RECEIVE_COMMAND(msg_buffer);

    EVT_ID_t evt_id[EVT_EVENTS_PER_SLOT] = {EVT_UNDEF};

    EVT_ID_t received_evt;

    if((strcmp(msg_buffer, "TO_NOMINAL") == 0))
    {
        received_evt = EVT_COMMS_TO_NOMINAL;
    }
    else if((strcmp(msg_buffer, "TO_SUN_SAFE") == 0))
    {
        received_evt = EVT_COMMS_TO_SUN_SAFE;
    }
    else if((strcmp(msg_buffer, "TO_CONTINGENCY") == 0))
    {
        received_evt = EVT_COMMS_TO_CONTINGENCY;
    }
    else if((strcmp(msg_buffer, "TO_COMMISSIONING") == 0))
    {
        received_evt = EVT_COMMS_TO_COMMISSIONING;
    }
    else
    {
        LOGGING("Received unknown command\n");
        return;
    }
    evt_id[0] = received_evt; 

    (void)obc_send_event_from_task(TASK_COMMS_ID, evt_id, 1);

   // vTaskDelayUntil(&xLastWakeTime, xPeriod);
    /* Leer a través del LoRa */
}

static void check_telecommand_rx(uint32_t *rx_tc)
{
    /*TODO implementation*/
    /* Check if there are telecommands received through LoRa and update the received telecommand */
}

static void process_telecommand(uint32_t rx_tc, EVT_ID_t *evt_id)
{
    /*TODO implementation*/
    /* Parse the received telecommand and get the corresponding event IDs*/
}

static HAL_StatusTypeDef UART7_ReceiveLine(char *buffer, uint16_t max_len)
{
    if (buffer == NULL || max_len == 0)
    {
        return HAL_ERROR;
    }

    uint16_t index = 0;
    uint8_t rx_char;

    while (index < (max_len - 1))
    {
        if (HAL_UART_Receive(&huart7,
                             &rx_char,
                             1,
                             HAL_MAX_DELAY) != HAL_OK)
        {
            return HAL_ERROR;
        }

        /* End of line */
        if (rx_char == '\n')
        {
            break;
        }

        buffer[index++] = (char)rx_char;
    }

    /* Null terminator */
    buffer[index] = '\0';

    return HAL_OK;
}