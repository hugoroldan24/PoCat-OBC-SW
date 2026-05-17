//Tx_proximity.c








#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>








#include "apps_common.h"
#include "apps_utilities.h"
#include "lr11xx_radio.h"
#include "lr11xx_regmem.h"
#include "lr11xx_system.h"
#include "smtc_hal_dbg_trace.h"
#include "uart_init.h"
















#include "protocol_definitions.h"// SDUFrame, PDU IDs, sizes
#include "frame_sublayer.h"      // serialize_sdu_frame(), deserialize_sdu_frame()
#include "io_sublayer.h"        // segment_sdu(), create_unfragmented_sdu(),
#include "dataser_sublayer.h"
#include "test_payload.h"








// USAR IMAGEN REAL
// Asegurate de haber generado imagen.h con el script python
#include "imagen.h"
#define CHUNK_SIZE 15000








static lr11xx_hal_context_t* context;








static uint8_t get_frame_fsn_for_print( const SDUFrame *frame);








static void print_sent_frame_queue_simple(const SentFrameQueue *queue);








static bool transmit_serialized_data(lr11xx_hal_context_t *context, SerializedData serialized);








static bool transmit_sent_frame_queue( lr11xx_hal_context_t *context, SentFrameQueue *queue);








static bool switch_tx_to_rx(lr11xx_hal_context_t *context, uint32_t rx_timeout_ms);








static bool wait_rx_done_or_timeout( lr11xx_hal_context_t *context);








static bool receive_sdu_frame(lr11xx_hal_context_t *context, SDUFrame *out_frame);








static void test_go_back_N(lr11xx_hal_context_t *context,const uint8_t *payload,  size_t payload_len);
static void test_go_back_N_large_payload(lr11xx_hal_context_t *context, const uint8_t *payload, size_t payload_len);
static void test_send_minimal_lora_packets(lr11xx_hal_context_t *context,uint32_t num_packets); //Solo enviar preambulo + synch word (y quizas algo mas, lo minimo posible)
static void send_preamble_bursts_sf9_bw800(lr11xx_hal_context_t *context,uint32_t num_chirps_preamble, uint32_t num_bursts); //Enviar solo preambulo, num_chirps_preamble es aproximado. Se caclula con la formula para SF=9, BW=800 (Importante transmitir con sf y bw)








static void send_payload_autofrag(lr11xx_hal_context_t *context, const uint8_t *payload, size_t payload_len);
static void send_infinite_preamble(lr11xx_hal_context_t *context);
















int main(void)
{
    /* Init MCU, shield, UART */
    smtc_hal_mcu_init();
    apps_common_shield_init();
    uart_init();








    HAL_DBG_TRACE_INFO("===== LR11xx TX PROXIMITY-1 PACKETS example =====\n\n");
    apps_common_print_sdk_driver_version();








    /* Init LR11xx context and radio */
    context = apps_common_lr11xx_get_context();
    apps_common_lr11xx_system_init((void*) context);
    apps_common_lr11xx_fetch_and_print_version((void*) context);
    apps_common_lr11xx_radio_init((void*) context);








    /* Example payload to send */
    // const uint8_t payload[] = "Hello from TX node FROM NANOSATLAB";
   
 
   
    // const size_t payload_len = sizeof(payload) - 1;
   //  HAL_DBG_TRACE_INFO("Full payload to send (len=%d): %s\n", (int)payload_len, payload);
    // HAL_DBG_TRACE_INFO("Prepared payload to send (len=%d): %s\n", (int)payload_len, payload);
    // HAL_DBG_TRACE_INFO("Prepared payload to send (len=%d bytes, will fragment into 3 segments)\n", (int)payload_len);
































    // Mensaje directo para generar 2 fragmentos: uno lleno (249) y otro parcial.
    // Total aprox 330 bytes.
    // static const uint8_t payload[] = ... (removed)








    // Large payload for stress test (>64KB)
    // #define LARGE_PAYLOAD_SIZE 70000
    // #define CHUNK_SIZE 15000 // Must be < NUM_MAX_SEGMENTS * MAX_FRAGMENTED_SDU_SIZE (64 * 249 = 15936)
   
    // static uint8_t large_payload[LARGE_PAYLOAD_SIZE];
   
    // // Initialize payload with a pattern
    // for (int i = 0; i < LARGE_PAYLOAD_SIZE; i++) {
    //     large_payload[i] = (uint8_t)(i % 256);
    // }








    // HAL_DBG_TRACE_INFO("Full payload to send (len=%d bytes)\n", LARGE_PAYLOAD_SIZE);








    /*
    // IMAGEN PAYLOAD (COMENTADO)
    HAL_DBG_TRACE_INFO("Full payload to send (len=%d bytes)\n", MY_IMAGE_SIZE);








    while (1) {
        HAL_DBG_TRACE_INFO("Starting transmission cycle of %d bytes...\n", MY_IMAGE_SIZE);
       
        size_t offset = 0;
        int chunk_index = 0;
       
        while (offset < MY_IMAGE_SIZE) {
            size_t current_chunk_size = CHUNK_SIZE;
            if (offset + current_chunk_size > MY_IMAGE_SIZE) {
                current_chunk_size = MY_IMAGE_SIZE - offset;
            }
           
            HAL_DBG_TRACE_INFO("Sending chunk %d (offset %d, size %d)...\n", chunk_index++, (int)offset, (int)current_chunk_size);
            send_payload_autofrag(context, &my_image_data[offset], current_chunk_size);
           
            offset += current_chunk_size;
           
            // Small delay between chunks to allow RX to process/print
            LL_mDelay(1000);
        }
       
        HAL_DBG_TRACE_INFO("Transmission cycle complete. Waiting 50s...\n\n");
        LL_mDelay(50000);
    }
    */








    // TEST PAYLOAD: 3 packets (A, B, C)
    // Assuming fragmentation size is 249 bytes (MAX_FRAGMENTED_SDU_SIZE)
    // Packet 1: 249 'A'
    // Packet 2: 249 'B'
    // Packet 3: 100 'C' (Incomplete/Partial)
























    /*
    #define FRAG_SIZE 249
    #define LAST_FRAG_SIZE 100
    #define TEST_SIZE (FRAG_SIZE * 2 + LAST_FRAG_SIZE)
   
    static uint8_t test_payload[TEST_SIZE];
    memset(test_payload, 'A', FRAG_SIZE);
    memset(test_payload + FRAG_SIZE, 'B', FRAG_SIZE);
    memset(test_payload + (FRAG_SIZE * 2), 'C', LAST_FRAG_SIZE);








    HAL_DBG_TRACE_INFO("Full payload to send (len=%d bytes) - ABC Pattern (Last partial)\n", TEST_SIZE);








    while (1) {
        send_payload_autofrag(context, test_payload, TEST_SIZE);
        HAL_DBG_TRACE_INFO("Transmission complete. Waiting 10s...\n\n");
        LL_mDelay(10000);
    }
















    */




    test_go_back_N_large_payload(context,my_image_data,MY_IMAGE_SIZE);
    //test_go_back_N(context, test_payload, test_payload_len); //IMPORTANTE PONER en apps_configuration CR,SF,BW
    //test_send_minimal_lora_packets(context, 32); //Poner CR a cero
    //send_preamble_bursts_sf9_bw800(context,64,32); //Bursts de 64 símbolos , 32 burst
    //send_infinite_preamble(context);








    return 0;
}
























//usamos un Helper: send payload with automatic fragmentation using io_sublayer helper
static void send_payload_autofrag(lr11xx_hal_context_t *context, const uint8_t *payload, size_t payload_len)
{
    HAL_DBG_TRACE_INFO(">>> ENTERED send_payload_autofrag\n");
    // Use static to avoid stack overflow (IOBuffer is ~65KB!)
    static IOBuffer buffer;
    create_buffer(&buffer); // Initialize by pointer - no stack copy!








    HAL_DBG_TRACE_INFO("Creating SDU frame (payload_len=%d, max_unfrag=%d)...\n", (int)payload_len, MAX_UNFRAGMENTED_SDU_SIZE);
   
    if (payload_len <= MAX_UNFRAGMENTED_SDU_SIZE) {
        // Create and enqueue an unfragmented SDU
        create_unfragmented_sdu((uint8_t*)payload, payload_len, 0 /*PortID*/, PDU_DATA, 0x0100 /*SC_ID*/, 0 /*SD_ID*/, &buffer);
        HAL_DBG_TRACE_INFO("Created unfragmented SDU\n");
    } else {
        // Create fragmented SDUs and append to buffer
        segment_sdu((uint8_t*)payload, payload_len, 0 /*PortID*/, PDU_DATA, 0x0100 /*SC_ID*/, 0 /*SD_ID*/, &buffer);
        HAL_DBG_TRACE_INFO("Created fragmented SDUs\n");
    }








    // Find the packet id we just created (should be the first in a fresh buffer)
    uint32_t packet_id = get_first_packet_id(&buffer);
        HAL_DBG_TRACE_INFO("Packet ID: 0x%08X\n", (unsigned int)packet_id);
   
    if (packet_id == UINT32_MAX) {
        HAL_DBG_TRACE_INFO("Error: no packet id found for payload\n");
        return;
    }








    // Get frames to send from next sublayer
    size_t frames_count = 0;
    static SDUFrame frames_to_send[NUM_MAX_SEGMENTS]; // Static buffer for output frames
   
    bool success = send_to_next_sublayer(&buffer, packet_id, frames_to_send, &frames_count, NUM_MAX_SEGMENTS);
    HAL_DBG_TRACE_INFO("send_to_next_sublayer returned %d frames\n", (int)frames_count);
   
    if (!success || frames_count == 0) {
        HAL_DBG_TRACE_INFO("Error: send_to_next_sublayer failed or zero count\n");
        return;
    }








    // Use send_to_LoRa which serializes using static buffer (no malloc in TX loop)
    int mul_count = (int)frames_count;
    HAL_DBG_TRACE_INFO("Starting transmission loop (%d segments)...\n", mul_count);
    while (mul_count > 0) {
        HAL_DBG_TRACE_INFO("Calling send_to_LoRa (remaining: %d)...\n", mul_count);
        SerializedData serialized = send_to_LoRa(frames_to_send, &mul_count);
        HAL_DBG_TRACE_INFO("send_to_LoRa returned: length=%d\n", (int)serialized.length);
       
        if (serialized.length == 0) {
            HAL_DBG_TRACE_INFO("Serialization/send failed for a segment\n");
            break;
        }








        // Debug print
        HAL_DBG_TRACE_INFO("Serialized segment (%d bytes): ", (int)serialized.length);
        for (size_t b = 0; b < serialized.length; ++b) {
            HAL_DBG_TRACE_PRINTF("%02X ", serialized.data[b]);
        }
        HAL_DBG_TRACE_PRINTF("\n");
         
        // Write to radio and TX (serialized.data points to static buffer, no free needed)
        apps_common_lr11xx_handle_pre_tx();
       
        // RE LEER Update packet parameters with actual payload length for this transmission
        lr11xx_radio_pkt_params_lora_t pkt_params = {
            .preamble_len_in_symb = LORA_PREAMBLE_LENGTH, // lo hace el lora
            .header_type          = LORA_PKT_LEN_MODE,
            .pld_len_in_bytes     = (uint8_t)serialized.length,  // Dynamic length
            .crc                  = LORA_CRC,
            .iq                   = LORA_IQ,
        };
        ASSERT_LR11XX_RC( lr11xx_radio_set_lora_pkt_params(context, &pkt_params) );
       
        // Write the actual bytes and transmit
        ASSERT_LR11XX_RC( lr11xx_regmem_write_buffer8(context, serialized.data, serialized.length) );
        ASSERT_LR11XX_RC( lr11xx_radio_set_tx( context, 0 ) );
       
        // WAIT FOR TX DONE (Critical for fragmentation timing)
        lr11xx_system_irq_mask_t irq_mask;
        do {
            ASSERT_LR11XX_RC(lr11xx_system_get_irq_status(context, &irq_mask));
        } while ((irq_mask & LR11XX_SYSTEM_IRQ_TX_DONE) == 0);
        ASSERT_LR11XX_RC(lr11xx_system_clear_irq_status(context, LR11XX_SYSTEM_IRQ_TX_DONE));








        HAL_DBG_TRACE_INFO("Segment sent over RF (%d bytes)\n", (int)serialized.length);








        // No free() needed - static buffer is reused








        // Delay entre segmentos: suficiente para que RX procese y vuelva a escuchar
        // Aumentado a 2000ms para asegurar que RX tiene tiempo de imprimir logs largos
        LL_mDelay(50);
    }
    // Clean buffer state
    free_buffer(&buffer, packet_id);
}
















static uint8_t get_frame_fsn_for_print(const SDUFrame *frame)
{
    if (frame->type == FRAME_FRAGMENTED) {
        return frame->data.fragmented.pdu_header.FSN;
    }








    return frame->data.unfragmented.header.FSN;
}
static void print_sent_frame_queue_simple(const SentFrameQueue *queue)
{
    if (queue == NULL) {
        return;
    }








    HAL_DBG_TRACE_INFO("\033[96mSentFrameQueue count=%d: ", (int)queue->count);








    for (size_t i = 0; i < queue->count; i++) {
        uint8_t fsn = get_frame_fsn_for_print(&queue->frames[i]);
        HAL_DBG_TRACE_PRINTF("| FSN %d ", (int)fsn);
    }








    HAL_DBG_TRACE_PRINTF("|\033[0m\n");
}




static bool transmit_sent_frame_queue(lr11xx_hal_context_t *context, SentFrameQueue *queue)
{
    if (context == NULL || queue == NULL || queue->count == 0) {
        return false;
    }








    SDUFrame frames_copy[DATASER_SENT_FRAME_QUEUE_SIZE];








    memcpy(frames_copy, queue->frames,sizeof(SDUFrame) * queue->count);








    int mul_count = (int)queue->count;








    while (mul_count > 0) {
        SerializedData serialized = send_to_LoRa(frames_copy, &mul_count);








        if (serialized.length == 0) {
            return false;
        }








        if (!transmit_serialized_data(context, serialized)) {
            return false;
        }








        LL_mDelay(50);
    }








    return true;
}




static bool switch_tx_to_rx(lr11xx_hal_context_t *context, uint32_t rx_timeout_ms)
{
    if (context == NULL) {
        return false;
    }








    HAL_DBG_TRACE_INFO("\033[94mSwitching radio from TX to RX mode...\033[0m\n");








    apps_common_lr11xx_handle_pre_rx();//Pone la led de rx del microcontrolador








    ASSERT_LR11XX_RC(lr11xx_radio_set_rx(context, rx_timeout_ms));








    return true;
}




static bool wait_rx_done_or_timeout(lr11xx_hal_context_t *context)
{
    if (context == NULL) {
        return false;
    }








    lr11xx_system_irq_mask_t irq_mask;








    do {
        ASSERT_LR11XX_RC(lr11xx_system_get_irq_status(context, &irq_mask));
    } while ((irq_mask & (LR11XX_SYSTEM_IRQ_RX_DONE | LR11XX_SYSTEM_IRQ_TIMEOUT)) == 0);








    ASSERT_LR11XX_RC(lr11xx_system_clear_irq_status(context, LR11XX_SYSTEM_IRQ_ALL_MASK));








    if (irq_mask & LR11XX_SYSTEM_IRQ_RX_DONE) {
        HAL_DBG_TRACE_INFO("\033[94mRX_DONE: PLCW/P-frame received\033[0m\n");
        return true;
    }








    HAL_DBG_TRACE_INFO("\033[91mRX timeout: no PLCW received\033[0m\n");
    return false;
}




static bool receive_sdu_frame(lr11xx_hal_context_t *context, SDUFrame *out_frame)
{
    if (context == NULL || out_frame == NULL) {
        return false;
    }








    uint8_t rx_buffer[255];
    uint8_t rx_size = 0;








    apps_common_lr11xx_handle_post_rx();








    apps_common_lr11xx_receive(
        context,
        rx_buffer,
        sizeof(rx_buffer),
        &rx_size
    );








    if (rx_size == 0) {
        return false;
    }








    *out_frame = deserialize_sdu_frame(rx_buffer);








    return check_sdu_frame(out_frame);
}




static bool transmit_serialized_data(lr11xx_hal_context_t *context, SerializedData serialized)
{
    if (context == NULL || serialized.length == 0) {
        return false;
    }


    apps_common_lr11xx_handle_pre_tx();


    lr11xx_radio_pkt_params_lora_t pkt_params = {
        .preamble_len_in_symb = LORA_PREAMBLE_LENGTH,
        .header_type          = LORA_PKT_LEN_MODE,
        .pld_len_in_bytes     = (uint8_t)serialized.length,
        .crc                  = LORA_CRC,
        .iq                   = LORA_IQ,
    };


    ASSERT_LR11XX_RC(lr11xx_radio_set_lora_pkt_params(context, &pkt_params));




    ASSERT_LR11XX_RC(lr11xx_regmem_write_buffer8(
        context,
        serialized.data,
        serialized.length
    ));




    ASSERT_LR11XX_RC(lr11xx_radio_set_tx(context, 0));


    lr11xx_system_irq_mask_t irq_mask;








    do {
        ASSERT_LR11XX_RC(lr11xx_system_get_irq_status(context, &irq_mask));
    } while ((irq_mask & LR11XX_SYSTEM_IRQ_TX_DONE) == 0);








    ASSERT_LR11XX_RC(lr11xx_system_clear_irq_status(context, LR11XX_SYSTEM_IRQ_TX_DONE));








    HAL_DBG_TRACE_INFO(
        "\033[92mSerialized frame sent over RF (%d bytes)\033[0m\n",
        (int)serialized.length
    );








    return true;
}










//Funcion go back N tx
static void test_go_back_N(lr11xx_hal_context_t *context,const uint8_t *payload,size_t payload_len){


    size_t test_payload_len = payload_len;


    HAL_DBG_TRACE_INFO(
        "\033[96mMessage to send (size=%d bytes)\033[0m\n", (int)test_payload_len);


    static IOBuffer io_buffer;
    static SentFrameQueue sent_queue;
    static FopPContext fop;
    uint32_t fop_timeout=15000;


    create_buffer(&io_buffer);
    sent_frame_queue_init(&sent_queue);
    fop_init(&fop, 0);//Timeout es el numero no lo utilizo aqui


    HAL_DBG_TRACE_INFO("\033[95mCreating transfer frames into IOBuffer...\033[0m\n");




    //IMPORTANTE el QoS esta en eXPEDITED POR PREDETERMINADO. no  influye en el test. Se tinee que cambiar.
    if (test_payload_len <= MAX_UNFRAGMENTED_SDU_SIZE) {


        create_unfragmented_sdu((uint8_t *)payload, test_payload_len,0, PDU_DATA,0x0100,0,&io_buffer);  // Payload,payload_len, PortID, U-frame:user data , SC_ID, SD_ID, iobufer
        HAL_DBG_TRACE_INFO("\033[95mCreated unfragmented transfer frame. IOBuffer size=%d\033[0m\n",(int)io_buffer.size);


    } else {
        segment_sdu((uint8_t *)payload,test_payload_len,0,PDU_DATA,0x0100,0,&io_buffer);// Payload,payload_len, PortID, U-frame:user data , SC_ID, SD_ID, iobufer


        HAL_DBG_TRACE_INFO("\033[95mCreated fragmented transfer frames. IOBuffer size=%d\033[0m\n",(int)io_buffer.size);
    }


    while (io_buffer.size > 0 || sent_queue.count > 0) { //Mientras en el iobuffer haya algo o en la sent queue algo
   
        // 1. Si hay huecos, relleno SentFrameQueue desde IOBuffer
        fill_sent_frame_queue_from_io_buffer(&io_buffer, &sent_queue, &fop);
        print_sent_frame_queue_simple(&sent_queue);


        // 2. Envio la ventana / SentFrameQueue
        bool frames_sent = transmit_sent_frame_queue(context, &sent_queue);


        // 3. Arranco timer si hay frames pendientes
        if (frames_sent && sent_queue.count > 0) {
            dataservice_timer_start(&fop.timer,0); //En el 0 estaba Halgettick


            HAL_DBG_TRACE_INFO("\033[93mTimer started: waiting for PLCW P-FRAME (timeout=%d ms, pending_frames=%d)\033[0m\n", (int)fop_timeout, (int)sent_queue.count);
        }




        // 4. Cambio a RX
        switch_tx_to_rx(context, fop_timeout); //tiempo máximo (15000) que el lora estara esperando algo
   
        // 5. Espero PLCW o timeout
        bool plcw_received = wait_rx_done_or_timeout(context);
            if (plcw_received) {
            // 6. Si llega PLCW: leer buffer, deserializar P-frame y procesar ACK


            SDUFrame received_pframe;


            if (receive_sdu_frame(context, &received_pframe)) {
                fop_process_plcw(&fop, &sent_queue, &io_buffer, &received_pframe); //lee el ACK, borra de la SentFrameQueue los frames ya confirmados, y rellena huecos con frames nuevos del IOBuffer
                HAL_DBG_TRACE_INFO("\033[93mPLCW processed, waiting before sending next FSN...\033[0m\n" );


                LL_mDelay(200);   // Espera para que el RX vuelva a ponerse en modo RX    


            }else{
                HAL_DBG_TRACE_INFO("\033[91mReceived frame could not be decoded as valid SDUFrame\033[0m\n");




            }


        } else {
            // 7. Si timeout: no borro la SentFrameQueue.
            // El while volvera a transmitir los frames pendientes.
            HAL_DBG_TRACE_INFO("\033[91mPLCW timeout: retransmitting SentFrameQueue (pending_frames=%d)\033[0m\n", (int)sent_queue.count);
            fop.snd_retx = true;
            dataservice_timer_stop(&fop.timer);




            }
     }




}








static void test_go_back_N_large_payload(lr11xx_hal_context_t *context, const uint8_t *payload, size_t payload_len){




    if(payload_len<=CHUNK_SIZE){
        test_go_back_N(context,payload, payload_len);
    }else{




        int num_full_chunks = payload_len/CHUNK_SIZE;
        int last_bytes=payload_len%CHUNK_SIZE;








        for(size_t i=0; i<num_full_chunks;i++){




            test_go_back_N(context,&payload[i*CHUNK_SIZE],CHUNK_SIZE);
            LL_mDelay(1000);




        }
        if(last_bytes>0){
             test_go_back_N(context,&payload[num_full_chunks*CHUNK_SIZE], last_bytes);
        }
       
    }
}




static void test_send_minimal_lora_packets( lr11xx_hal_context_t *context, uint32_t num_packets){
    lr11xx_radio_pkt_params_lora_t pkt_params = {
        .preamble_len_in_symb = LORA_PREAMBLE_LENGTH,
        .header_type          = LR11XX_RADIO_LORA_PKT_IMPLICIT,
        .pld_len_in_bytes     = 0,
        .crc                  = LR11XX_RADIO_LORA_CRC_OFF,
        .iq                   = LORA_IQ,
    };








    for (uint32_t i = 0; i < num_packets; i++)
    {
        lr11xx_system_irq_mask_t irq_status;








        HAL_DBG_TRACE_INFO(
            "Sending minimal LoRa packet %lu/%lu (\033[94mpreamble=%d\033[0m symbols, payload=\033[94m0\033[0m)...\n",
            (unsigned long)(i + 1),
            (unsigned long)num_packets,
            (int)LORA_PREAMBLE_LENGTH
        );








        apps_common_lr11xx_handle_pre_tx();








        ASSERT_LR11XX_RC( lr11xx_radio_set_lora_pkt_params( context, &pkt_params ) );
        ASSERT_LR11XX_RC( lr11xx_radio_set_tx( context, 0 ) );








        while (1)
        {
            ASSERT_LR11XX_RC(
                lr11xx_system_get_and_clear_irq_status( context, &irq_status )
            );








            if ( ( irq_status & LR11XX_SYSTEM_IRQ_TX_DONE ) == LR11XX_SYSTEM_IRQ_TX_DONE )
            {
                HAL_DBG_TRACE_INFO(
                    "\033[93mPacket \033[94m%lu\033[93m sent.\033[0m\n",
                    (unsigned long)(i + 1)
                );
                break;
            }
        }








        LL_mDelay(200);
    }
}
static void send_preamble_bursts_sf9_bw800(lr11xx_hal_context_t *context,uint32_t num_chirps_preamble, uint32_t num_bursts)
{
    uint32_t burst_time_ms;








    /* Caso fijo: SF9, BW800 kHz
       Ts = 2^9 / 812000 ≈ 0.63 ms por chirp */
    burst_time_ms = (num_chirps_preamble * 63) / 100;








    if (burst_time_ms == 0)
    {
        burst_time_ms = 1;
    }








    HAL_DBG_TRACE_INFO(
        "\033[95mSF=SF9  BW=BW800  CR=NO_CR\033[0m\n"
    );








    for (uint32_t i = 0; i < num_bursts; i++)
    {
        HAL_DBG_TRACE_INFO(
            "Sending preamble burst %lu/%lu (\033[94mchirps=%lu\033[0m)...\n",
            (unsigned long)(i + 1),
            (unsigned long)num_bursts,
            (unsigned long)num_chirps_preamble
        );








        apps_common_lr11xx_handle_pre_tx();
        ASSERT_LR11XX_RC( lr11xx_radio_set_tx_infinite_preamble( context ) );








        LL_mDelay( burst_time_ms );








        ASSERT_LR11XX_RC(
            lr11xx_system_set_standby( context, LR11XX_SYSTEM_STANDBY_CFG_RC )
        );








        HAL_DBG_TRACE_INFO(
            "\033[93mBurst \033[94m%lu\033[93m sent.\033[0m\n",
            (unsigned long)(i + 1)
        );








        LL_mDelay(200);
    }
}
static void send_infinite_preamble(lr11xx_hal_context_t *context)
{
    HAL_DBG_TRACE_INFO("\033[95mSending infinite LoRa preamble...\033[0m\n");








    apps_common_lr11xx_handle_pre_tx();
    ASSERT_LR11XX_RC( lr11xx_radio_set_tx_infinite_preamble( context ) );








    while (1)
    {
    }
}



















































