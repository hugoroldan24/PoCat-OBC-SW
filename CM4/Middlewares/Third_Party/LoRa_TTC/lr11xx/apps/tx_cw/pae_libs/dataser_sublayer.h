#ifndef DATASER_SUBLAYER_H
#define DATASER_SUBLAYER_H


#include "protocol_definitions.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "io_sublayer.h"




//Implementación nueva ---------------------------------------------------------------------


typedef enum {
    INACTIVE = 0,
    CONNECTING_T,
    CONNECTING_L,
    ACTIVE
} DataServiceMode;


typedef enum {
    FULL = 0,
    HALF,
    SIMPLEX_TRANSMIT,
    SIMPLEX_RECEIVE
} DataServiceDuplexMode;




//------------TX-----------------------------------------------------------------------------------------------


#define DATASER_SENT_FRAME_QUEUE_SIZE 6//Size of the SentFrameQueue = maximum transfer-frames sent without acknowledge them


typedef struct{
    uint32_t start_ms;
    uint32_t timeout_ms;
    bool active;
}DataServiceTimer;


typedef struct {
    uint8_t snd_sndNxt;      // V(S): next Sequence Controlled FSN to send
    uint8_t snd_rcvNxt;      // N(R): current PLCW Report Value
    uint8_t snd_prevRcvNxt;  // NN(R): previous valid PLCW Report Value


    bool snd_retx;           // R(R): current PLCW Retransmit Flag
    bool snd_prevRetx;       // RR(R): previous PLCW Retransmit Flag


    DataServiceTimer timer;


} FopPContext;


typedef struct {
    SDUFrame frames[DATASER_SENT_FRAME_QUEUE_SIZE];
    size_t count; //How many transfer-frames there are in the SentFrameQueue right now
} SentFrameQueue;     //Sent Frame Queue contains Sequence Controlled frames that have been sent but not acknowledged by the receiver.








void sent_frame_queue_init(SentFrameQueue *queue); //Crear e inicializar frame sent queue


bool sent_frame_queue_remove_acked_fsn(SentFrameQueue *queue, uint8_t report_value); //Mira si el numAck-1 es igual a algun fsn, si es cierto lo quita de la cola, se quita moviendo a la izquierda los demás. Devuelve si se ha quitado uno no


bool io_buffer_to_sent_frame_queue(IOBuffer *buffer,SentFrameQueue *queue, FopPContext *fop);
void dataservice_set_fsn(uint8_t fsn, SDUFrame *frame); //Helper de io_buffer_to_sent_frame_queue






void dataservice_timer_init(DataServiceTimer *timer, uint32_t timeout_ms);
void fop_init(FopPContext *fop, uint32_t timeout_ms);


void dataservice_timer_start(DataServiceTimer *timer, uint32_t now_ms);


void dataservice_timer_stop(DataServiceTimer *timer);


bool dataservice_timer_expired(const DataServiceTimer *timer, uint32_t now_ms);






//------------RX---------------------------------------------------------------------------------------------------------






typedef struct {
    uint8_t rcv_rcvNxt;      // V(R): next Sequence Controlled FSN expected
    bool rcv_retx;           // Retransmit Flag to report in PLCW
    bool receiver_ready;     // Receiver ready/status flag
} FarmPContext;






// Type F1 SPDU: Proximity Link Control Word (16 bits)
// SPDU Header: bits 0-1
// SPDU Data Field: bits 2-15
typedef struct __attribute__((packed)) { //- Define la estructura del PLCW como una SPDU fija de 16 bits. Se usa `packed` para evitar padding del compilador y bitfields para representar campos de 1, 3 y 8 bits según el Blue Book.


    // SPDU Header
    uint8_t SPDU_Format_ID : 1;          // Bit 0: 1 = fixed-length SPDU
    uint8_t SPDU_Type_ID : 1;            // Bit 1: 0 = PLCW


    // SPDU Data Field
    uint8_t Retransmit_Flag : 1;         // Bit 2
    uint8_t PCID : 1;                    // Bit 3
    uint8_t Reserved_Spare : 1;          // Bit 4, shall be 0
    uint8_t Expedited_Frame_Counter : 3; // Bits 5-7
    uint8_t Report_Value : 8;            // Bits 8-15
} PLCW;






SDUFrame dataservice_create_plcw_pframe(    
    uint8_t report_value,
    uint8_t expedited_frame_counter,
    uint8_t pcid,
    bool retransmit_flag,
    uint8_t PortID,
    uint16_t SC_ID,
    uint8_t SD_ID
);


//Tx-------------------------------




bool dataservice_get_report_value(const SDUFrame *pframe, uint8_t *report_value);


// --------RX------------------------------------------------------------------------------


void farmp_process_transfer_frame(FarmPContext *farmp, const SDUFrame *transfer_frame);




bool farmp_process_frame_and_create_plcw(
    FarmPContext *farmp,
    const SDUFrame *received_frame,
    uint8_t expedited_frame_counter,
    uint8_t pcid,
    uint8_t PortID,
    uint16_t SC_ID,
    uint8_t SD_ID,
    SDUFrame *out_plcw_pframe
);


void farmp_init(FarmPContext *farmp);
//---------------TX--------------------------------




size_t fill_sent_frame_queue_from_io_buffer(IOBuffer *buffer, SentFrameQueue *queue, FopPContext *fop);




bool fop_process_plcw(
    FopPContext *fop,
    SentFrameQueue *queue,
    IOBuffer *buffer,
    const SDUFrame *pframe
);








#endif // DATASER_SUBLAYER_H



