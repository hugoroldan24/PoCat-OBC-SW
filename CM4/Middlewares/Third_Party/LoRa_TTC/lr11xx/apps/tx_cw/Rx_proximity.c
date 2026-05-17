







#include <stdio.h>
#include <string.h>
// #include <stdlib.h> // Removed
#include <stdint.h>
















#include "apps_common.h"
#include "apps_utilities.h"
#include "lr11xx_radio.h"
#include "lr11xx_system.h"
#include "smtc_hal_dbg_trace.h"
#include "uart_init.h"
















#include "protocol_definitions.h"   // SDUFrame
#include "frame_sublayer.h"         // deserialize_sdu_frame(), check_sdu_frame()
#include "io_sublayer.h"
#include "dataser_sublayer.h"
#include "lr11xx_regmem.h"
















static lr11xx_hal_context_t* context;
static void receive_and_process(lr11xx_hal_context_t *context);
static void free_sdu_frame(SDUFrame* frame);




















static void go_back_N(lr11xx_hal_context_t *context);
static void test_go_back_N(lr11xx_hal_context_t *context);
static bool switch_to_rx(lr11xx_hal_context_t *context, uint32_t rx_timeout_ms);
static bool wait_rx_done_or_timeout(lr11xx_hal_context_t *context);
static void print_received_payload(const SDUFrame *frame);
static bool transmit_serialized_data(lr11xx_hal_context_t *context, SerializedData serialized);








static void test_receive_minimal_lora_packets(lr11xx_hal_context_t *context);
static void receive_preamble_bursts_sf9_bw800(lr11xx_hal_context_t *context,uint32_t num_chirps_preamble);
































// Buffer estático para reensamblado
// SerializedData already contains storage
static uint8_t reassembly_data[16000];  //16384
static uint32_t reassembly_len = 0;








int main(void)
{
    smtc_hal_mcu_init();
    apps_common_shield_init();
    uart_init();
















    HAL_DBG_TRACE_INFO("===== LR11xx RX PROXIMITY-1 PACKETS EXAMPLE =====\n");
    apps_common_print_sdk_driver_version();
















    context = apps_common_lr11xx_get_context();
    apps_common_lr11xx_system_init((void*) context);
    apps_common_lr11xx_fetch_and_print_version((void*) context);
    apps_common_lr11xx_radio_init((void*) context); //Aquí se pone el CR, BW, SF




    //test_go_back_N(context);
    go_back_N(context);
    //test_receive_minimal_lora_packets(context);
    //receive_preamble_bursts_sf9_bw800(context, 64);








    /*








    while (1)
    {
        receive_and_process(context);
        LL_mDelay(10);  // Delay mínimo para volver a RX rápidamente
    }
    */
   
    return 0;
}








/*
static uint32_t calc_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else         crc >>= 1;
        }
    }
    return ~crc;
}
*/








static void receive_and_process(lr11xx_hal_context_t *context)
{
    uint8_t rx_buffer[255];
    uint8_t rx_size = 0;
















    apps_common_lr11xx_handle_pre_rx();
    ASSERT_LR11XX_RC(lr11xx_radio_set_rx(context, 10000));  // RX con timeout 10s (suficiente para recibir 3 segmentos)
















    // HAL_DBG_TRACE_INFO("Waiting for incoming LoRa packet...\n");  // Comentado para no saturar UART
















    // Wait for IRQ (RX done or timeout)
    lr11xx_system_irq_mask_t irq_mask;
    do {
        ASSERT_LR11XX_RC(lr11xx_system_get_irq_status(context, &irq_mask));
    } while ((irq_mask & (LR11XX_SYSTEM_IRQ_RX_DONE | LR11XX_SYSTEM_IRQ_TIMEOUT)) == 0);








    ASSERT_LR11XX_RC(lr11xx_system_clear_irq_status(context, LR11XX_SYSTEM_IRQ_ALL_MASK));








    if (irq_mask & LR11XX_SYSTEM_IRQ_RX_DONE)
    {
        apps_common_lr11xx_handle_post_rx();
        apps_common_lr11xx_receive(context, rx_buffer, sizeof(rx_buffer), &rx_size);
























        HAL_DBG_TRACE_PRINTF("Raw: ");
        for (int i = 0; i < rx_size; i++) {
            HAL_DBG_TRACE_PRINTF("%02X ", rx_buffer[i]);
        }
        HAL_DBG_TRACE_PRINTF("\n");
















        /* ========= DESERIALIZAR FRAME ========= */
        SDUFrame frame = deserialize_sdu_frame(rx_buffer);
















        if (check_sdu_frame(&frame))
        {
            PDUHeader* hdr = NULL;
            uint8_t* payload = NULL;
            uint16_t length = 0;
















            /* Determinar tipo */
            if (frame.type == FRAME_UNFRAGMENTED)
            {
                hdr = &frame.data.unfragmented.header;
                payload = frame.data.unfragmented.sdu;
                HAL_DBG_TRACE_INFO("  Type: Unfragmented\n");
            }
            else if (frame.type == FRAME_FRAGMENTED)
            {
                hdr = &frame.data.fragmented.pdu_header;
                payload = frame.data.fragmented.sdu;
                HAL_DBG_TRACE_PRINTF("FRAG FSN=%d Seg=%d PID=%d | ",
                    hdr->FSN,
                    frame.data.fragmented.seg_header.SegFlag,
                    frame.data.fragmented.seg_header.PseudoPacketID);
            }
            else
            {
                HAL_DBG_TRACE_WARNING("Unknown frame type.\n");
               
                return;
            }
















            /* Longitud real del SDU */
            length = ((uint16_t)hdr->data_length_high << 8) | hdr->data_length_low;
















            /* ========= IMPRIMIR HEADER ========= */
            HAL_DBG_TRACE_INFO("Frame header:");
            HAL_DBG_TRACE_PRINTF("  Version: %d\n", hdr->VersionNum);
            HAL_DBG_TRACE_PRINTF("  QoS: %d\n", hdr->QoS);
            HAL_DBG_TRACE_PRINTF("  PDU_ID: %d\n", hdr->PDU_ID);
            HAL_DBG_TRACE_PRINTF("  DFC_ID: %d\n", hdr->DFC_ID);
            HAL_DBG_TRACE_PRINTF("  PortID: %d\n", hdr->PortID);
            HAL_DBG_TRACE_PRINTF("  SD_ID: %d\n", hdr->SD_ID);
            HAL_DBG_TRACE_PRINTF("  PC_ID: %d\n", hdr->PC_ID);
















            uint16_t scid = ((uint16_t)hdr->SC_ID_part1 << 8) | hdr->SC_ID_part2;
            HAL_DBG_TRACE_PRINTF("  SC_ID: 0x%04X\n", scid);
















            HAL_DBG_TRACE_PRINTF("  FSN: %d\n", hdr->FSN);
            HAL_DBG_TRACE_PRINTF("  SDU length: %d bytes\n", length);
















            /* ========= REENSAMBLADO DE FRAGMENTOS ========= */
            if (frame.type == FRAME_FRAGMENTED)
            {
                // Serializar fragmento actual al buffer de reensamblado
                serialize_to_obc(frame, reassembly_data, &reassembly_len, sizeof(reassembly_data));
               
                HAL_DBG_TRACE_INFO("Fragment added to reassembly buffer (total: %d bytes)\n",
                                   (int)reassembly_len);
               
                // Verificar si necesitamos más segmentos
                if (need_more_seg(frame))
                {
                    HAL_DBG_TRACE_INFO("Waiting for more segments...\n");
                    free_sdu_frame(&frame);
                    // NO hacer return - continuar en RX en el mismo ciclo del while(1)
                }
                else
                {
                    HAL_DBG_TRACE_INFO("=== REASSEMBLY COMPLETE ===\n");
                    HAL_DBG_TRACE_INFO("Total payload size: %d bytes\n", (int)reassembly_len);
                   
                    /*
                    // Verificar CRC
                    uint32_t calculated_crc = calc_crc32(reassembly_data, reassembly_len);
                    HAL_DBG_TRACE_INFO("CRC32 Calculado: 0x%08X\n", (unsigned int)calculated_crc);
                    */
                   
                    // Imprimir payload completo reensamblado en HEXADECIMAL para reconstrucción
                    //HAL_DBG_TRACE_INFO("Complete payload (HEX):\n");
                    //for (size_t i = 0; i < reassembly_len; i++)
                    //{
                    //    HAL_DBG_TRACE_PRINTF("%02X", reassembly_data[i]);
                    //}
                    //HAL_DBG_TRACE_PRINTF("\n\n");








                   
                    // Imprimir payload completo reensamblado (ASCII) - COMENTADO
                    HAL_DBG_TRACE_INFO("Complete payload (ASCII):\n");
                    for (size_t i = 0; i < reassembly_len; i++)
                    {
                        char c = reassembly_data[i];
                        if (c >= 32 && c <= 126) // Caracteres imprimibles
                            HAL_DBG_TRACE_PRINTF("%c", c);
                        else
                            HAL_DBG_TRACE_PRINTF(".");
                    }
                    HAL_DBG_TRACE_PRINTF("\n\n");
                   
                   
                    // Limpiar buffer de reensamblado
                    memset(reassembly_data, 0, sizeof(reassembly_data));
                    reassembly_len = 0;
                }
            }
            else
            {
                /* ========= PAYLOAD UNFRAGMENTED ========= */
                if (payload != NULL && length > 0)
                {
                    HAL_DBG_TRACE_INFO("Payload (ASCII):\n");
                    for (int j = 0; j < length; j++)
                    {
                        char c = payload[j];
                        if (c >= 32 && c <= 126)
                            HAL_DBG_TRACE_PRINTF("%c", c);
                        else
                            HAL_DBG_TRACE_PRINTF(".");
                    }
                    HAL_DBG_TRACE_PRINTF("\n");
                }
            }
           
            free_sdu_frame(&frame);
        }
        else
        {
            HAL_DBG_TRACE_WARNING("Received frame is invalid or corrupted.\n");
        }
    }
    else
    {
        HAL_DBG_TRACE_INFO("RX timeout: no packet received.\n");
    }
}




static void free_sdu_frame(SDUFrame* frame)
{
    // No dynamic memory to free
}




static bool switch_to_rx(lr11xx_hal_context_t *context, uint32_t rx_timeout_ms)
{
    if (context == NULL) {
        return false;
    }








    HAL_DBG_TRACE_INFO("\033[94mSwitching radio to RX mode...\033[0m\n");








    apps_common_lr11xx_radio_init_without_prints(context); // RESTAURA params LoRa, payload 255, CRC, etc.
    apps_common_lr11xx_handle_pre_rx();








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
        HAL_DBG_TRACE_INFO("\033[94mRX_DONE: transfer frame received\033[0m\n");
        return true;
    }








    HAL_DBG_TRACE_INFO("\033[91mRX timeout: no transfer frame received\033[0m\n");
    return false;
}




static void print_received_payload(const SDUFrame *frame)
{
    if (frame == NULL) {
        return;
    }








    if (frame->type == FRAME_FRAGMENTED) {
        serialize_to_obc(
            *frame,
            reassembly_data,
            &reassembly_len,
            sizeof(reassembly_data)
        );








        HAL_DBG_TRACE_INFO(
            "\033[96mFragment received. Reassembly size=%d bytes\033[0m\n",
            (int)reassembly_len
        );








        /*
        if (!need_more_seg(*frame)) {
            HAL_DBG_TRACE_INFO("\033[96mComplete payload received:\033[0m\n");








            for (size_t i = 0; i < reassembly_len; i++) {
                char c = reassembly_data[i];








                if (c >= 32 && c <= 126) {
                    HAL_DBG_TRACE_PRINTF("%c", c);
                } else {
                    HAL_DBG_TRACE_PRINTF(".");
                }
            }








            HAL_DBG_TRACE_PRINTF("\n");








            memset(reassembly_data, 0, sizeof(reassembly_data));
            reassembly_len = 0;
        }




        */
    if (!need_more_seg(*frame)) {




        HAL_DBG_TRACE_PRINTF("IMGHEX:");
        for (size_t i = 0; i < reassembly_len; i++) {
            HAL_DBG_TRACE_PRINTF("%02X", reassembly_data[i]);
        }
        HAL_DBG_TRACE_PRINTF("\n");




        memset(reassembly_data, 0, sizeof(reassembly_data));
        reassembly_len = 0;
    }












    PDUHeader *header = (PDUHeader *)&frame->data.unfragmented.header;








    uint16_t length =
        ((uint16_t)header->data_length_high << 8) |
        header->data_length_low;




    /*
    HAL_DBG_TRACE_INFO("\033[96mPayload received:\033[0m\n");








    for (uint16_t i = 0; i < length; i++) {
        char c = frame->data.unfragmented.sdu[i];








        if (c >= 32 && c <= 126) {
            HAL_DBG_TRACE_PRINTF("%c", c);
        } else {
            HAL_DBG_TRACE_PRINTF(".");
        }
    }
    HAL_DBG_TRACE_PRINTF("\n"); */
    }
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
        "\033[93mPLCW P-frame sent over RF (%d bytes)\033[0m\n",
        (int)serialized.length
    );








    return true;
}






























static void test_go_back_N(lr11xx_hal_context_t *context)
{
    static FarmPContext farmp;
    farmp_init(&farmp);








    while (1) {








    // 1. Poner radio en RX
    switch_to_rx(context, 30000);
    // 2. Esperar frame
    bool frame_received = wait_rx_done_or_timeout(context);








    if (frame_received) {
        // 3. Recibir bytes
        uint8_t rx_buffer[255]; //Buffer temporal donde se guardan los bytes que llegan por radio
        uint8_t rx_size = 0;








        apps_common_lr11xx_handle_post_rx();








        apps_common_lr11xx_receive( context, rx_buffer,sizeof(rx_buffer), &rx_size); //Pasan los bytes recibidos del chip LR11xx a rx_buffer. rx_size es cuantos bytes llegan
















        // 4. Deserializar SDUFrame
        SDUFrame received_frame = deserialize_sdu_frame(rx_buffer); //Ahora tenemos el transfer-frame




















        if (received_frame.type == FRAME_FRAGMENTED) {
            HAL_DBG_TRACE_INFO(
                "\033[96mRX deserialized fragmented FSN=%d PDU_ID=%d\033[0m\n",
                received_frame.data.fragmented.pdu_header.FSN,
                received_frame.data.fragmented.pdu_header.PDU_ID
            );
        } else {
            HAL_DBG_TRACE_INFO(
                "\033[96mRX deserialized unfragmented FSN=%d PDU_ID=%d\033[0m\n",
                received_frame.data.unfragmented.header.FSN,
                received_frame.data.unfragmented.header.PDU_ID
            );
        }
















       
        // 5. FARM-P procesa frame y crea plcw
       








        if (check_sdu_frame(&received_frame)) { //Comprobamos si el transfer-frame recibido es válido








            print_received_payload(&received_frame); //Aqui se printea y se mete en reassembly data








            SDUFrame plcw_pframe; //variable auxiliar








            if (farmp_process_frame_and_create_plcw(&farmp, &received_frame, 0, 0,0, 0x0100, 0, &plcw_pframe )) {








                HAL_DBG_TRACE_INFO("\033[92mFARM-P processed frame and created PLCW P-frame\033[0m\n" );








                SDUFrame tx_frames[1]; //cREAMOS ESTO PORQUE SEND_TO_LORA ESTA DISEÑADA PARA UN ARRAY DE SDUFRAMES (yo envio frame a frame)
                tx_frames[0] = plcw_pframe;








                int frame_count = 1;
                SerializedData serialized = send_to_LoRa(tx_frames, &frame_count);








                if (serialized.length > 0) {
                    //HAL_DBG_TRACE_INFO("\033[93mWaiting 200ms before sending PLCW...\033[0m\n");
                    LL_mDelay(600);








                    transmit_serialized_data(context, serialized);
                } else {
                    HAL_DBG_TRACE_INFO("\033[91mError serializing PLCW P-frame\033[0m\n");
                }
            }
        } else {
            HAL_DBG_TRACE_INFO("\033[91mInvalid SDUFrame received\033[0m\n");
        }
















    } else {
        HAL_DBG_TRACE_INFO("\033[91mNo frame received, returning to RX...\033[0m\n");
    }
























    }
}




static void go_back_N(lr11xx_hal_context_t *context)
{
    static FarmPContext farmp;
    farmp_init(&farmp);
    while(1){
        uint32_t between_frames_timeout = 1000;
        uint32_t first_frame_timeout = 8000;
        bool burst_has_valid_frame = false;
        bool burst_started=false;
        SDUFrame plcw_pframe; // PLCW acumulativo a enviar cuando cierre la rafaga




        while (1) {
            uint32_t current_timeout;
            if(burst_started){
                current_timeout=between_frames_timeout;
            }else{
                current_timeout=first_frame_timeout;
            }
            // Esperamos un nuevo frame con timeout corto entre frames de la misma rafaga o si es el primero de una rafaga lo esperamos mas
            switch_to_rx(context, current_timeout);
            bool frame_received = wait_rx_done_or_timeout(context);




            if (frame_received) { //Si llega un frame
                burst_started=true;




                uint8_t rx_buffer[255];
                uint8_t rx_size = 0;
               




                apps_common_lr11xx_handle_post_rx();
                apps_common_lr11xx_receive(context, rx_buffer, sizeof(rx_buffer), &rx_size);




                // Deserializamos el transfer-frame recibido
                SDUFrame received_frame = deserialize_sdu_frame(rx_buffer);




                if (received_frame.type == FRAME_FRAGMENTED) {
                    HAL_DBG_TRACE_INFO(
                        "\033[96mRX deserialized fragmented FSN=%d PDU_ID=%d\033[0m\n",
                        received_frame.data.fragmented.pdu_header.FSN,
                        received_frame.data.fragmented.pdu_header.PDU_ID
                    );
                } else {
                    HAL_DBG_TRACE_INFO(
                        "\033[96mRX deserialized unfragmented FSN=%d PDU_ID=%d\033[0m\n",
                        received_frame.data.unfragmented.header.FSN,
                        received_frame.data.unfragmented.header.PDU_ID
                    );
                }




                if (check_sdu_frame(&received_frame)) {
                    // El frame es valido: lo procesamos y actualizamos FARM-P
                    print_received_payload(&received_frame);
                    farmp_process_transfer_frame(&farmp, &received_frame);
                    burst_has_valid_frame = true;




                    HAL_DBG_TRACE_INFO(
                        "\033[92mValid frame processed. Current cumulative Report Value=%d\033[0m\n",
                        farmp.rcv_rcvNxt
                    );
                } else {
                    // El frame no es valido: no lo usamos para avanzar secuencia
                    HAL_DBG_TRACE_INFO("\033[91mInvalid SDUFrame received\033[0m\n");
                }
            } else {
                // Ha vencido el tiempo entre frames
                if (burst_has_valid_frame) {
                    // Cerramos la rafaga y generamos un unico PLCW acumulativo
                    plcw_pframe = dataservice_create_plcw_pframe(
                        farmp.rcv_rcvNxt,   // Report Value acumulativo = FSN+1 esperado
                        0,                  // expedited_frame_counter
                        0,                  // pcid
                        farmp.rcv_retx,     // retransmit_flag
                        0,                  // PortID
                        0x0100,             // SC_ID
                        0                   // SD_ID
                    );




                    HAL_DBG_TRACE_INFO(
                        "\033[92mInter-frame timeout. Sending cumulative PLCW with Report Value=%d\033[0m\n",
                        farmp.rcv_rcvNxt
                    );




                    SDUFrame tx_frames[1];
                    tx_frames[0] = plcw_pframe;




                    int frame_count = 1;
                    SerializedData serialized = send_to_LoRa(tx_frames, &frame_count);




                    if (serialized.length > 0) {
                        transmit_serialized_data(context, serialized);
                    } else {
                        HAL_DBG_TRACE_INFO("\033[91mError serializing cumulative PLCW P-frame\033[0m\n");
                    }




                    // Ya hemos contestado a esta rafaga, salimos para volver al bucle exterior
                    break;
                } else {
                    // No llego ningun frame valido en esta espera: no hay nada que contestar
                    HAL_DBG_TRACE_INFO(
                        "\033[93mInter-frame timeout with no valid frame received. Continuing RX...\033[0m\n"
                    );
                    break;
                }
            }
        }        
    }








}






static void test_receive_minimal_lora_packets(lr11xx_hal_context_t *context)
{
    uint32_t received_packets = 0;








    lr11xx_radio_pkt_params_lora_t pkt_params = {
        .preamble_len_in_symb = LORA_PREAMBLE_LENGTH,
        .header_type          = LR11XX_RADIO_LORA_PKT_IMPLICIT,
        .pld_len_in_bytes     = 0,
        .crc                  = LR11XX_RADIO_LORA_CRC_OFF,
        .iq                   = LORA_IQ,
    };








    ASSERT_LR11XX_RC( lr11xx_radio_set_lora_pkt_params( context, &pkt_params ) );








    while (1)
    {
        lr11xx_system_irq_mask_t irq_status;








        apps_common_lr11xx_handle_pre_rx();
        ASSERT_LR11XX_RC( lr11xx_radio_set_rx( context, 0 ) );








        while (1)
        {
            ASSERT_LR11XX_RC(
                lr11xx_system_get_and_clear_irq_status( context, &irq_status )
            );








            if ( ( irq_status & LR11XX_SYSTEM_IRQ_RX_DONE ) == LR11XX_SYSTEM_IRQ_RX_DONE )
            {
                lr11xx_radio_pkt_status_lora_t packet_status;
                ASSERT_LR11XX_RC( lr11xx_radio_get_lora_pkt_status( context, &packet_status ) );








                received_packets++;








                HAL_DBG_TRACE_INFO(
                    "\033[93mPacket \033[94m%lu\033[93m received. \033[95mRSSI = %d dBm\033[0m\n",
                    (unsigned long)received_packets,
                    (int)packet_status.rssi_pkt_in_dbm
                );








                break;
            }








            if ( ( irq_status & LR11XX_SYSTEM_IRQ_TIMEOUT ) == LR11XX_SYSTEM_IRQ_TIMEOUT )
            {
                HAL_DBG_TRACE_INFO("\033[91mRX timeout.\033[0m\n");
                break;
            }
        }








        LL_mDelay(100);
    }
}
static void receive_preamble_bursts_sf9_bw800(lr11xx_hal_context_t *context,uint32_t num_chirps_preamble)
{
    uint32_t burst_count = 0;
    uint32_t burst_time_ms;








    /* SF9, BW800 kHz -> Ts ≈ 0.64 ms por chirp */
    burst_time_ms = (num_chirps_preamble * 64) / 100;








    if (burst_time_ms == 0)
    {
        burst_time_ms = 1;
    }








    apps_common_lr11xx_handle_pre_rx();
    ASSERT_LR11XX_RC( lr11xx_radio_set_rx( context, 0xFFFFFF ) );








    while (1)
    {
        int8_t rssi_inst;








        LL_mDelay(1);








        ASSERT_LR11XX_RC( lr11xx_radio_get_rssi_inst( context, &rssi_inst ) );








        if (rssi_inst > -80)
        {
            burst_count++;








            HAL_DBG_TRACE_INFO(
                "\033[93mBurst \033[94m%lu\033[93m detected. \033[95mRSSI = %d dBm\033[0m\n",
                (unsigned long)burst_count,
                (int)rssi_inst
            );








            /* Espera a que pase ese burst para no contarlo varias veces */
            LL_mDelay(burst_time_ms + 150);
        }
    }
}



































