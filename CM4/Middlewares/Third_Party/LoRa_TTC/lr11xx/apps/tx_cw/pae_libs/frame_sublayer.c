#include "frame_sublayer.h"
#include "protocol_definitions.h"
#include "io_sublayer.h"

#include <string.h>
#include <stdio.h>
// #include <stdlib.h> // Removed

// Data transmission


// Serialize SDU frame into a caller-provided buffer (no malloc)
size_t serialize_into(const SDUFrame* frame, uint8_t* buffer, size_t buffer_size) {
    if (!frame || !buffer || buffer_size == 0) {
        return 0; // Invalid input
    }

    uint32_t sdu_length = 0;
    uint32_t total_length = 0;

    // Calculate the SDU length according to the frame type
    if (frame->type == FRAME_UNFRAGMENTED) {
        sdu_length = ((uint32_t)frame->data.unfragmented.header.data_length_high << 8) | 
                     frame->data.unfragmented.header.data_length_low;
        total_length = sizeof(PDUHeader) + sdu_length;
    } else { // FRAGMENTED
        sdu_length = ((uint32_t)frame->data.fragmented.pdu_header.data_length_high << 8) | 
                     frame->data.fragmented.pdu_header.data_length_low;
        total_length = sizeof(PDUHeader) + sizeof(SegmentationHeader) + sdu_length;
    }

    // Check if buffer is large enough
    if (total_length > buffer_size) {
        return 0; // Buffer too small
    }

    uint32_t offset = 0;

    // Copy headers and SDU according to the type
    if (frame->type == FRAME_UNFRAGMENTED) {
        memcpy(buffer + offset, &frame->data.unfragmented.header, sizeof(PDUHeader));
        offset += sizeof(PDUHeader);
        memcpy(buffer + offset, frame->data.unfragmented.sdu, sdu_length);
    } else {
        memcpy(buffer + offset, &frame->data.fragmented.pdu_header, sizeof(PDUHeader));
        offset += sizeof(PDUHeader);
        memcpy(buffer + offset, &frame->data.fragmented.seg_header, sizeof(SegmentationHeader));
        offset += sizeof(SegmentationHeader);
        memcpy(buffer + offset, frame->data.fragmented.sdu, sdu_length);
    }

    return total_length;
}
// Choose the priority of the data to send
void choose_priority(SDUFrame* MultiplexedData, int* count, int max_capacity, SDUFrame newData) {
    if (MultiplexedData == NULL || count == NULL) {
        return;
    }

    if (*count >= max_capacity) {
        return; // Buffer full
    }

    int pos = *count; // Default append

    if(newData.type == FRAME_UNFRAGMENTED && newData.data.unfragmented.header.PDU_ID == PDU_COMMAND){
        // Insert before first DATA PDU
        for(int i = 0; i < *count; i++){
            if (MultiplexedData[i].type == FRAME_UNFRAGMENTED && 
                MultiplexedData[i].data.unfragmented.header.PDU_ID == PDU_DATA) {
                pos = i;
                break;
            }
        }
    }

    // Shift elements
    for (int i = *count; i > pos; i--) {
        MultiplexedData[i] = MultiplexedData[i - 1];
    }

    // Insert
    MultiplexedData[pos] = newData;
    (*count)++;
}

bool check_data(SDUFrame* MultiplexedData, int* count) {
    if (MultiplexedData == NULL || *count == 0) {
        return false; 
    }
    return true; 
}
// Send the first data from the multiplexed data list
SerializedData send_to_LoRa(SDUFrame* MultiplexedData, int* count) {
    SerializedData to_send = {{0}, 0}; 
    
    if (MultiplexedData == NULL || count == NULL || *count == 0) {
        return to_send; 
    }

    // Serialize the first SDUFrame directly into the return structure
    size_t tx_len = serialize_into(&MultiplexedData[0], to_send.data, sizeof(to_send.data));
    
    if (tx_len == 0) {
        return to_send; 
    }
    to_send.length = tx_len;

    // Shift the remaining elements left by one
    if (*count > 1) {
        memmove(MultiplexedData, MultiplexedData + 1, sizeof(SDUFrame) * ((*count) - 1));
    }
    
    memset(&MultiplexedData[*count - 1], 0, sizeof(SDUFrame));
    (*count)--;

    return to_send;
}


// Data reception
// Create the SDUFrame from the byte sequence
SDUFrame deserialize_sdu_frame(const uint8_t* data) {
    SDUFrame frame = {0};
    if (!data) {
        return frame; // Error: null data
    }

    // Store the first 5 bytes in the PDU header
    PDUHeader pdu_header;
     // Byte 0
     //uint8_t aux = data[0];
     pdu_header.VersionNum = data[0] & 0x03;               // bits 0-1
     pdu_header.QoS        = (data[0] >> 2) & 0x01;         // bit 2
     pdu_header.PDU_ID     = (data[0] >> 3) & 0x01;         // bit 3
     pdu_header.DFC_ID     = (data[0] >> 4) & 0x03;         // bits 4-5
     pdu_header.SC_ID_part1= (data[0] >> 6) & 0x03;         // bits 6-7
 
     // Byte 1
     pdu_header.SC_ID_part2 = data[1];                     // 8 direct bits
 
     // Byte 2
     pdu_header.PC_ID           = data[2] & 0x01;           // bit 0
     pdu_header.PortID          = (data[2] >> 1) & 0x07;     // bits 1-3
     pdu_header.SD_ID           = (data[2] >> 4) & 0x01;     // bit 4
     pdu_header.data_length_high = (data[2] >> 5) & 0x07;    // bits 5-7
 
     // Byte 3
     pdu_header.data_length_low = data[3];                 // 8 direct bits
 
     // Byte 4
     pdu_header.FSN = data[4];                             // 8 direct bits

    // Calculate the SDU length; it will only be the low 8 bits since LoRa limits the size and larger packets will not arrive
    uint8_t sdu_length = pdu_header.data_length_low;

    // Check if the SDU is fragmented or not
    if(pdu_header.DFC_ID == DFC_FRAGMENTED) {
        frame.type = FRAME_FRAGMENTED;
        frame.data.fragmented.pdu_header = pdu_header;
        frame.data.fragmented.seg_header.SegFlag = (data[5] & 0x03); // bits 0-1
        frame.data.fragmented.seg_header.PseudoPacketID = (data[5] >> 2) & 0x3F; // bits 2-7
        
        // Static copy
        if (sdu_length <= MAX_FRAGMENTED_SDU_SIZE) {
             memcpy(frame.data.fragmented.sdu, data + (SIZE_SEGMENTATION_HEADER+SIZE_PDU_HEADER), sdu_length);
        }
    } else {
        frame.type = FRAME_UNFRAGMENTED;
        frame.data.unfragmented.header = pdu_header;
        
        // Static copy
        if (sdu_length <= MAX_UNFRAGMENTED_SDU_SIZE) {
            memcpy(frame.data.unfragmented.sdu, data + SIZE_PDU_HEADER, sdu_length);
        }
    }

    return frame;
}

bool check_sdu_frame(const SDUFrame* frame) {
    if (!frame) {
        return false; // Error: null frame
    }

    // Check if the SDUFrame has valid data
    if (frame->type == FRAME_UNFRAGMENTED) {
        if (frame->data.unfragmented.sdu == NULL ||
            frame->data.unfragmented.header.VersionNum != VERSION_3) { // VERSION_3 = 0b10 = 2
            return false; // Error: null SDU
        }
    } else { // FRAME_FRAGMENTED
        if (frame->data.fragmented.sdu == NULL ||
            frame->data.fragmented.pdu_header.VersionNum != VERSION_3) {
            return false; // Error: null SDU_f
        }
    }
    
    return true; // Valid SDUFrame
}