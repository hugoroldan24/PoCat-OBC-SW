// Include standard input/output library
#include <stdio.h>
// Include string manipulation library
#include <string.h>
// Include standard library
#include <stdlib.h>

// Include application-specific common functions
#include "apps_common.h"
// Include application-specific utilities
#include "apps_utilities.h"
// Include LR11xx radio driver
#include "lr11xx_radio.h"
// Include LR11xx system driver
#include "lr11xx_system.h"
// Include LR11xx system types
#include "lr11xx_system_types.h"
// Include LR11xx register and memory driver
#include "lr11xx_regmem.h"
// Include main header for ping-pong application
#include "main_ping_pong.h"
// Include debug trace functionality
#include "smtc_hal_dbg_trace.h"
// Include UART initialization functions
#include "uart_init.h"

// Define address of the device
#define ADRSS  1
// Define destination address
#define DEST_ADRSS 2
// Define length of the header
#define HEADER_LENGTH 4
// Define delay before packet transmission in milliseconds
#define DELAY_BEFORE_TX_MS 20
// Define delay between each ping-pong activity in milliseconds
#define DELAY_PING_PONG_PACE_MS 200
// Define LR11xx interrupt mask used by the application
#define IRQ_MASK                                                                          \
    ( LR11XX_SYSTEM_IRQ_TX_DONE | LR11XX_SYSTEM_IRQ_RX_DONE | LR11XX_SYSTEM_IRQ_TIMEOUT | \
      LR11XX_SYSTEM_IRQ_HEADER_ERROR | LR11XX_SYSTEM_IRQ_CRC_ERROR | LR11XX_SYSTEM_IRQ_FSK_LEN_ERROR )

// Define whether the application uses partial sleep mode
#define APP_PARTIAL_SLEEP true
// Define maximum payload length excluding header length
#define MAX_PAYLOAD_LENGTH PAYLOAD_LENGTH-HEADER_LENGTH

// Declare context for LR11xx HAL
static lr11xx_hal_context_t* context;
// Declare flag for receive mode
static bool    rx_mode = false;
// Declare flag for availability
static bool    avaliable = true;
// Declare index for transmission
uint8_t index = 0;
// Declare index for reception
uint8_t rx_index = 0;

// Define large message to be transmitted
static const uint8_t large_msg[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 
    62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 
    92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 
    118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 
    142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 
    166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 
    190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200};
// Declare buffer for received message (size matches large_msg)
static uint8_t rx_message[sizeof( large_msg )];
// Define size of the large message
static size_t large_msg_size = sizeof( large_msg );

// Define frame structure using union
typedef union {
    uint8_t raw[PAYLOAD_LENGTH];
    struct
    {
        uint8_t dst_addr; //destination address
        uint8_t src_addr; //source address
        uint8_t type;  //if 0x00 is ACK frame if 0x01 is a data frame if  0x02 is a end of tx frame 
                       // if 0x03 is a tx request if 0x04 is a tx acceptance
        uint8_t length; //length of the data
        uint8_t data[MAX_PAYLOAD_LENGTH]; //data
    } frame_t; 
} frame;

// Declare global variables for frame
frame my_variable_global; //to be able to print the data
frame my_variable_rx;

/* Forward declarations for functions implemented below but used before their
    definitions (prevents implicit declaration errors) */
void tx_request( uint8_t dst );
void tx_accepted( uint8_t dst );
void info_tx( uint8_t* large_msg, uint8_t index, uint8_t size, uint8_t dest );
void end_of_tx( uint8_t dest );
void ack_tx( uint8_t dest );

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/**
 * @brief Handle reception failure for ping-pong example
 */
static void reception_failure_handling( void );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

/**
 * @brief Main application entry point.
 */
int main( void )
{
    uint8_t tx_size;

    // Initialize MCU
    smtc_hal_mcu_init( );
    // Initialize shield
    apps_common_shield_init( );
    // Initialize UART
    uart_init();

    // Print initial message
    HAL_DBG_TRACE_INFO( "===== Link communications EXAMPLE =====\n\n" );
    // Print SDK driver version
    apps_common_print_sdk_driver_version( );

    // Get context for LR11xx
    context = apps_common_lr11xx_get_context( );

    // Initialize LR11xx system
    apps_common_lr11xx_system_init( ( void* ) context );
    // Fetch and print LR11xx version
    apps_common_lr11xx_fetch_and_print_version( ( void* ) context );
    // Initialize LR11xx radio
    apps_common_lr11xx_radio_init( ( void* ) context );

    // Set DIO IRQ parameters
    ASSERT_LR11XX_RC( lr11xx_system_set_dio_irq_params( context, IRQ_MASK, 0 ) );
    // Clear IRQ status
    ASSERT_LR11XX_RC( lr11xx_system_clear_irq_status( context, LR11XX_SYSTEM_IRQ_ALL_MASK ) );

    // Initialize random number generator
    srand( 10 );

    // Send transmission request
    tx_request(DEST_ADRSS);

    // Main loop
    while( true )
    {
        // Process IRQ
        apps_common_lr11xx_irq_process( context, IRQ_MASK );
    }
}

// Function to handle transmission done event
void on_tx_done( void )
{
    // Handle post transmission
    apps_common_lr11xx_handle_post_tx( );
    
    // Print transmitted message
    HAL_DBG_TRACE_INFO("Transmitted message: ");
    for (int i = 0; i < PAYLOAD_LENGTH; i++) {
        HAL_DBG_TRACE_PRINTF("%02X ", my_variable_global.raw[i]);
    }
    HAL_DBG_TRACE_PRINTF("\n");

    // Handle pre-reception
    apps_common_lr11xx_handle_pre_rx( );
    // Set radio to receive mode with random delay
    ASSERT_LR11XX_RC( lr11xx_radio_set_rx(context,get_time_on_air_in_ms( ) + RX_TIMEOUT_VALUE + rand( ) % 500 ) );  // Random delay to avoid
                                                                                                                    // unwanted synchronization
}

// Function to handle reception done event
void on_rx_done( void )
{
    uint8_t buffer_rx[PAYLOAD_LENGTH]; // Buffer to store received data
    uint8_t size, tx_size; // Variables to store size of received and transmitted data
    
    // Handle post reception
    apps_common_lr11xx_handle_post_rx( );

    // Receive data
    apps_common_lr11xx_receive( context, buffer_rx, PAYLOAD_LENGTH, &size );
    // Parse received frame
    my_variable_rx.frame_t.dst_addr = buffer_rx[0]; // Destination address
    my_variable_rx.frame_t.src_addr = buffer_rx[1]; // Source address
    my_variable_rx.frame_t.type = buffer_rx[2]; // Frame type
    my_variable_rx.frame_t.length = buffer_rx[3]; // Data length
    if(my_variable_rx.frame_t.type == 0x01){ // If frame type is data
        memcpy( my_variable_rx.frame_t.data, &buffer_rx[4], my_variable_rx.frame_t.length ); // Copy data
    }

    // Check if received address matches
    if(my_variable_rx.frame_t.dst_addr != ADRSS){
        HAL_DBG_TRACE_INFO("Received address different: %d != %d", my_variable_rx.frame_t.dst_addr,ADRSS); // Log address mismatch
        reception_failure_handling( ); // Handle reception failure
    } else {
        // Check if received size matches
        if(size != PAYLOAD_LENGTH){
            HAL_DBG_TRACE_INFO( "Received size different: %d != %d", size, PAYLOAD_LENGTH); // Log size mismatch
            reception_failure_handling( ); // Handle reception failure
        } else {    
            // Handle different frame types
            if(my_variable_rx.frame_t.type == 0x03){ // If frame type is tx request
                HAL_DBG_TRACE_INFO("Tx request received"); // Log tx request
                if(avaliable ){
                    avaliable = false; // Set availability to false
                    rx_mode=true; // Set receive mode to true
                    tx_accepted(my_variable_rx.frame_t.src_addr); // Transmit acceptance
                }
            }
            if(my_variable_rx.frame_t.type == 0x04) // If frame type is tx acceptance
            {
                HAL_DBG_TRACE_INFO("Tx acceptance received"); // Log tx acceptance
                avaliable = false; // Set availability to false
                tx_size = (large_msg_size - index < MAX_PAYLOAD_LENGTH ) ? large_msg_size - index : MAX_PAYLOAD_LENGTH; // Calculate tx size
                info_tx(large_msg, index, tx_size, my_variable_rx.frame_t.src_addr); // Transmit information
            }

            if( my_variable_rx.frame_t.type == 0x00) // If frame type is ack
            {
                HAL_DBG_TRACE_INFO("ACK received"); // Log ack
                if(rx_mode){ // This is the case of the ACK of an end of transmission
                    rx_mode=false; // Set receive mode to false
                    avaliable=true; // Set availability to true
                    if(index < large_msg_size ){
                        tx_request(DEST_ADRSS); // Transmit request
                    } else {
                        HAL_DBG_TRACE_INFO("No information to transmit"); // Log no information to transmit
                        apps_common_lr11xx_handle_pre_rx( ); // Handle pre-reception
                        ASSERT_LR11XX_RC( lr11xx_radio_set_rx(context,get_time_on_air_in_ms( ) + RX_TIMEOUT_VALUE + rand( ) % 500 ) ); // Set radio to receive mode
                    }
                } else { // This is the case of the ACK of a data transmission
                    index += MAX_PAYLOAD_LENGTH; // Increment index
                    if( index < large_msg_size )
                    {                
                        tx_size = (large_msg_size - index < MAX_PAYLOAD_LENGTH ) ? large_msg_size - index : MAX_PAYLOAD_LENGTH; // Calculate tx size
                        info_tx(large_msg, index, tx_size,my_variable_rx.frame_t.src_addr); // Transmit information
                    } else {
                        end_of_tx(my_variable_rx.frame_t.src_addr); // Transmit end of tx
                    }
                }
            }

            if (my_variable_rx.frame_t.type == 0x01) // If frame type is data
            {
                HAL_DBG_TRACE_INFO("Data received"); // Log data received
                memcpy( &rx_message[rx_index], my_variable_rx.frame_t.data, my_variable_rx.frame_t.length ); // Copy received data
                rx_index += my_variable_rx.frame_t.length; // Increment rx index
                ack_tx(my_variable_rx.frame_t.src_addr); // Transmit acknowledgment
            }
            if (my_variable_rx.frame_t.type == 0x02) // If frame type is end of transmission
            {
                if(rx_mode){
                    HAL_DBG_TRACE_INFO("End of transmission received"); // Log end of transmission received
                    end_of_tx(my_variable_rx.frame_t.src_addr); // Transmit end of tx
                    HAL_DBG_TRACE_INFO("\n\n Full received message: "); // Log full received message
                } else {
                    HAL_DBG_TRACE_INFO("End of transmission confirmation"); // Log end of transmission confirmation
                    avaliable=true; // Set availability to true
                    ack_tx(my_variable_rx.frame_t.src_addr); // Transmit acknowledgment
                }
            }
        }
    }
}

// Function to handle reception timeout event
void on_rx_timeout( void )
{
    reception_failure_handling( );
}

// Function to handle CRC error event
void on_rx_crc_error( void )
{
    reception_failure_handling( );
}

// Function to handle FSK length error event
void on_fsk_len_error( void )
{
    reception_failure_handling( );
}

// Function to handle reception failure
/**
 * @brief Handles the reception failure scenario.
 *
 * This function is called when a reception failure occurs. It performs the necessary
 * actions to handle the failure, including retransmission and setting up the receiver
 * for the next attempt.
 *
 * The function performs the following steps:
 * 1. Calls `apps_common_lr11xx_handle_post_rx` to handle post-reception tasks.
 * 2. Checks if data is available and if the current index is less than the size of the large message.
 *    - If true, it requests transmission to the destination address.
 *    - Otherwise, it prepares for the next reception by calling `apps_common_lr11xx_handle_pre_rx` and
 *      setting the receiver with a timeout.
 * 3. If data is not available and the receiver is not in receive mode:
 *    - Logs a retransmission message.
 *    - If the current index is less than the size of the large message, it calculates the transmission size
 *      and calls `info_tx` to transmit the data.
 *    - Otherwise, it calls `end_of_tx` to signal the end of transmission.
 * 4. If the receiver is in receive mode:
 *    - Checks if the received frame type is 0x02 (ACK confirmation lost).
 *      - If true, logs a message and calls `end_of_tx` to retransmit the end of transmission.
 *    - Prepares for the next reception by calling `apps_common_lr11xx_handle_pre_rx` and setting the receiver
 *      with a timeout.
 */
static void reception_failure_handling( void )
{
    uint8_t tx_size;
    // Handle post reception
    apps_common_lr11xx_handle_post_rx( );
    if(avaliable==true){
        if(index < large_msg_size ){
            tx_request(DEST_ADRSS);
        } else {
            apps_common_lr11xx_handle_pre_rx( );
            ASSERT_LR11XX_RC( lr11xx_radio_set_rx(context,get_time_on_air_in_ms( ) + RX_TIMEOUT_VALUE + rand( ) % 500 ) );
        }
    } else {
        if(!rx_mode){
            HAL_DBG_TRACE_INFO( "Retransmission\n\n" );
            if( index < large_msg_size )
            {                
                tx_size = (large_msg_size - index < MAX_PAYLOAD_LENGTH ) ? large_msg_size - index : MAX_PAYLOAD_LENGTH;  
                info_tx(large_msg, index, tx_size,my_variable_rx.frame_t.src_addr);
            } else {
                end_of_tx(my_variable_rx.frame_t.src_addr); 
            }
        } else {
            if(my_variable_rx.frame_t.type == 0x02){    //if the ACK confirmation is lost, the end of transmission is retransmitted
                HAL_DBG_TRACE_INFO("End of transmission retransmission, ACK needed");
                end_of_tx(my_variable_rx.frame_t.src_addr);
            }
            apps_common_lr11xx_handle_pre_rx( );
            ASSERT_LR11XX_RC( lr11xx_radio_set_rx(context,get_time_on_air_in_ms( ) + RX_TIMEOUT_VALUE + rand( ) % 500 ) ); 
        }
    }
}

// Function to transmit information
void info_tx(uint8_t* large_msg, uint8_t index, uint8_t size, uint8_t dest){
    frame my_variable;
    my_variable.frame_t.src_addr = ADRSS;
    my_variable.frame_t.dst_addr = dest;
    my_variable.frame_t.type = 1;
    my_variable.frame_t.length = size;
    for(int i = 0; i < size; i++){
        my_variable.frame_t.data[i] = large_msg[index + i];
    }
    
    my_variable_global=my_variable;
    HAL_DBG_TRACE_INFO( "Info transmission\n\n" );
    ASSERT_LR11XX_RC(lr11xx_regmem_write_buffer8(context, my_variable.raw, PAYLOAD_LENGTH) );
    apps_common_lr11xx_handle_pre_tx( );
    ASSERT_LR11XX_RC( lr11xx_radio_set_tx( context, 0 ) );
}

// Function to transmit acknowledgment
void ack_tx(uint8_t dest){
    frame my_variable;
    my_variable.frame_t.src_addr = ADRSS;
    my_variable.frame_t.dst_addr = dest;
    my_variable.frame_t.type = 0;
    my_variable.frame_t.length = 0;
    my_variable_global=my_variable;
    HAL_DBG_TRACE_INFO( "ACK transmission\n\n" );
    ASSERT_LR11XX_RC(lr11xx_regmem_write_buffer8(context, my_variable.raw, HEADER_LENGTH) );
    apps_common_lr11xx_handle_pre_tx( );
    ASSERT_LR11XX_RC( lr11xx_radio_set_tx( context, 0 ) );
}

// Function to transmit end of transmission
void end_of_tx(uint8_t dest){
    frame my_variable;
    my_variable.frame_t.src_addr = ADRSS;
    my_variable.frame_t.dst_addr = dest;
    my_variable.frame_t.type = 2;
    my_variable.frame_t.length = 0;
    my_variable_global=my_variable;
    HAL_DBG_TRACE_INFO( "End of tx transmission\n\n");
    ASSERT_LR11XX_RC(lr11xx_regmem_write_buffer8(context, my_variable.raw, HEADER_LENGTH) );
    apps_common_lr11xx_handle_pre_tx( );
    ASSERT_LR11XX_RC( lr11xx_radio_set_tx( context, 0 ) );
}

// Function to transmit request
void tx_request(uint8_t dst){
    frame my_variable;
    my_variable.frame_t.src_addr =ADRSS;
    my_variable.frame_t.dst_addr = dst;
    my_variable.frame_t.type = 3;
    my_variable.frame_t.length = 0;
    my_variable_global=my_variable;
    HAL_DBG_TRACE_INFO( "Tx request transmission\n\n" );
    ASSERT_LR11XX_RC(lr11xx_regmem_write_buffer8(context, my_variable.raw , HEADER_LENGTH) );
    apps_common_lr11xx_handle_pre_tx( );
    ASSERT_LR11XX_RC( lr11xx_radio_set_tx( context, 0 ) );
}

// Function to transmit acceptance
void tx_accepted(uint8_t dst){
    frame my_variable;
    my_variable.frame_t.src_addr = ADRSS;
    my_variable.frame_t.dst_addr = dst;
    my_variable.frame_t.type = 4;
    my_variable.frame_t.length = 0;
    my_variable_global=my_variable;
    HAL_DBG_TRACE_INFO( "Tx acceptance transmission\n\n" );
    ASSERT_LR11XX_RC(lr11xx_regmem_write_buffer8(context, my_variable.raw, HEADER_LENGTH) );
    apps_common_lr11xx_handle_pre_tx( );
    ASSERT_LR11XX_RC( lr11xx_radio_set_tx( context, 0 ) );
}