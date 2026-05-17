#include "protocol_definitions.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "io_sublayer.h"
#include "dataser_sublayer.h"
#include <string.h>








//Crear e inicalizar SentFrameQueue
void sent_frame_queue_init(SentFrameQueue *queue){


    if (queue == NULL) { //por si alguien llama a la funcion sin querer
    return;
    }


    queue->count = 0;
    memset(queue->frames, 0, sizeof(queue->frames));
}




//Inicailizar valores timer (es inicializar el struct del DataServiceTimer) (Helper de fop_init) Elegimos el timeout
void dataservice_timer_init(DataServiceTimer *timer, uint32_t timeout_ms)
{
    if (timer == NULL) {
        return;
    }
    timer->start_ms = 0;
    timer->timeout_ms = timeout_ms;
    timer->active = false;
}


//Inicializar el Todo lo del FOP-P (FopPcontext)
void fop_init(FopPContext *fop, uint32_t timeout_ms)
{
    if (fop == NULL) {
        return;
    }


    fop->snd_sndNxt = 0;
    fop->snd_rcvNxt = 0;
    fop->snd_prevRcvNxt = 0;


    fop->snd_retx = false;
    fop->snd_prevRetx = false;


    dataservice_timer_init(&fop->timer, timeout_ms);
}


//Que empieze el timer en now_ms --> Eso se hará en el main con **HAL_GetTick()**
void dataservice_timer_start(DataServiceTimer *timer, uint32_t now_ms){


    if(timer==NULL){
        return;
    }
    timer->start_ms=now_ms;
    timer->active=true;




}




//Funcion que pare el timer porque ya ha llegado el PLCW (ACK)
void dataservice_timer_stop(DataServiceTimer *timer){
    if(timer==NULL){
        return;
    }
    timer->active=false;
}




//Te dice si ha ya ha caducado el timer (osea si ha llegado timeout). Le pasas con **HAL_GetTick()** el now_ms(tiempo actual)
bool dataservice_timer_expired(const DataServiceTimer *timer, uint32_t now_ms)
{
    if (timer == NULL || !timer->active) { //Si el timer es null(no le pasamo parametro) o el timer no esta activo no hacemos nada
        return false;
    }


    return (now_ms - timer->start_ms) >= timer->timeout_ms;  //Si lo que lleva el timer corriendo es mayor o igual al timeout devolvemos true(expired)
}






//helper para sent_frame_queue_remove_acked_fsn : te dice si tiene que borrar el frame porque el numACK es mayor (ackeado)
//static porque solo lo utilizaremos dentor de dataser_sublayer.c
static bool fsn_is_acknowledged(uint8_t fsn, uint8_t report_value)
{
    uint8_t distance = report_value - fsn;


    return distance > 0 && distance <= 128; //Protección con el 128. Puede dar el caso por ejemplo que FSN=254. Y llega Report Value=0. (como es uint8_t el maximo es 255 depsues es cero)Entonces si que esta ackeado. COmo eligiremos una ventana no muy grande seguramente pues funcionará.
    //Con una ventana de 129 por ejemplo si puede fallar (ambiguidad)
}




//Sem ira en la sent_frame_queue los ackeados y se borran. Devulve true si se ha ackeado 1 o más.
bool sent_frame_queue_remove_acked_fsn(SentFrameQueue *queue, uint8_t report_value)
{
    if (queue == NULL) {
        return false;
    }


    if (queue->count == 0) {
        return false;
    }


    bool removed = false;
    size_t i = 0;


    while (i < queue->count) { //Mientras estemos en un transfer-frame de la sent-frame-queue
        uint8_t frame_fsn;


        if (queue->frames[i].type == FRAME_FRAGMENTED) {
            frame_fsn = queue->frames[i].data.fragmented.pdu_header.FSN;
        } else {
            frame_fsn = queue->frames[i].data.unfragmented.header.FSN;
        }


        if (fsn_is_acknowledged(frame_fsn, report_value)) { //Comprobamos si el report_value lo ha ackeado
            // Borramos el frame ACKeado.
            for (size_t j = i; j + 1 < queue->count; j++) {
                queue->frames[j] = queue->frames[j + 1];
            }


            queue->count--; //Habrá uno menos


            // Limpiamos la ultima posicion duplicada.
            memset(&queue->frames[queue->count], 0, sizeof(SDUFrame));


            removed = true;


            // No incrementamos i porque el siguiente frame se ha desplazado a esta posicion.
        } else {
            i++; //Incrementamos i, miramos el siguiente frame (si hay)
        }
    }


    return removed; //Devolvemos si se ha ackeado alguno (minimo 1 si es true, pueden haber sido más)
}


//Cuando expire el timer tendre que volver a enviar los frames


// Helper de io_buffer_to_sent_frame_queue: asigna FSN a un SDUFrame
void dataservice_set_fsn(uint8_t fsn, SDUFrame *frame)
{
    if (frame == NULL) {
        return;
    }


    if (frame->type == FRAME_FRAGMENTED) {
        frame->data.fragmented.pdu_header.FSN = fsn;
    } else {
        frame->data.unfragmented.header.FSN = fsn;
    }
}


//tAMBINE SE asinga el FSN con el Helper (funcion de arriba). Solo rellena a la sent_frame_queue con 1.
bool io_buffer_to_sent_frame_queue(IOBuffer *buffer,SentFrameQueue *queue, FopPContext *fop){ //OJO NO INDEX DEL IOBUFFER ESTARA MAL; PORQUE YO PASO DEL BUFFER A LA QUEUE FRAME A FRAME (No TOCO index)


    if (buffer == NULL || queue == NULL ||fop==NULL) {
        return false;
    }


    if (buffer->size == 0) {
        return false;
    }


    if (queue->count >= DATASER_SENT_FRAME_QUEUE_SIZE) {
        return false;
    }
    // Copiamos el primer frame del IOBuffer a la SentFrameQueue
    queue->frames[queue->count] = buffer->frames[0];


    // Asignamos el FSN real de FOP-P al frame que queda en SentFrameQueue
    dataservice_set_fsn(fop->snd_sndNxt, &queue->frames[queue->count]);
   
    fop->snd_sndNxt++; //Incrementamos el snd_sndNxt
    queue->count++;






    // Borramos buffer->frames[0] desplazando todo el IOBuffer a la izquierda
    for (size_t i = 0; i + 1 < buffer->size; i++) {
        buffer->frames[i] = buffer->frames[i + 1];
    }


    buffer->size--;


    // Limpiamos la ultima posicion duplicada
    memset(&buffer->frames[buffer->size], 0, sizeof(SDUFrame));






    return true;


}


//Lo usmaos para llenar la sentframequeue. Utuiiza   io_buffer_to_sent_frame_queue tantas veces como huecos hyaa. Para poder transmitir la ventana
size_t fill_sent_frame_queue_from_io_buffer(IOBuffer *buffer, SentFrameQueue *queue, FopPContext *fop)
{
    size_t added = 0;


    if (buffer == NULL || queue == NULL || fop == NULL) {
        return 0;
    }


    while (queue->count < DATASER_SENT_FRAME_QUEUE_SIZE && buffer->size > 0) {
        if (!io_buffer_to_sent_frame_queue(buffer, queue, fop)) {
            break;
        }


        added++;
    }


    return added;
}




//Crea PLCW lógico constuye estructura de 16bits. Es el Helper de dataservice
static PLCW dataservice_create_plcw(uint8_t report_value, uint8_t expedited_frame_counter,  uint8_t pcid,  bool retransmit_flag)
{
    PLCW plcw = {0};


    // SPDU Header
    plcw.SPDU_Format_ID = 1; // fixed-length SPDU
    plcw.SPDU_Type_ID = 0;   // PLCW


    // SPDU Data Field
    plcw.Retransmit_Flag = retransmit_flag ? 1 : 0;
    plcw.PCID = pcid & 0x01;
    plcw.Reserved_Spare = 0;
    plcw.Expedited_Frame_Counter = expedited_frame_counter & 0x07;
    plcw.Report_Value = report_value;


    return plcw;
}




SDUFrame dataservice_create_plcw_pframe(    
    uint8_t report_value,
    uint8_t expedited_frame_counter,
    uint8_t pcid,
    bool retransmit_flag,
    uint8_t PortID,
    uint16_t SC_ID,
    uint8_t SD_ID
)
{
    SDUFrame frame = {0};


    PLCW plcw = dataservice_create_plcw(
        report_value,
        expedited_frame_counter,
        pcid,
        retransmit_flag
    );


    frame.type = FRAME_UNFRAGMENTED;


    PDUHeader pdu_header = create_pdu_header(
        VERSION_3,
        EXPEDITED,
        PDU_COMMAND,
        DFC_PACKETS,
        SC_ID,
        PRIMARYCANAL,
        PortID,
        SD_ID,
        sizeof(PLCW),
        0
    );


    frame.data.unfragmented.header = pdu_header;
    memcpy(frame.data.unfragmented.sdu, &plcw, sizeof(PLCW));


    return frame;
}


//Pasa el report value por puntero. Decuelve true si se ha heoc satisfacotriamente.
bool dataservice_get_report_value(const SDUFrame *pframe, uint8_t *report_value)
{
    if (pframe == NULL || report_value == NULL) {
        return false;
    }


    if (pframe->type != FRAME_UNFRAGMENTED) {
        return false;
    }


    if (pframe->data.unfragmented.header.PDU_ID != PDU_COMMAND) {
        return false;
    }


    PLCW plcw = {0};
    memcpy(&plcw, pframe->data.unfragmented.sdu, sizeof(PLCW));


    if (plcw.SPDU_Format_ID != 1 || plcw.SPDU_Type_ID != 0) {
        return false;
    }


    *report_value = plcw.Report_Value;


    return true;
}




void farmp_process_transfer_frame(FarmPContext *farmp, const SDUFrame *transfer_frame)
{
    if (farmp == NULL || transfer_frame == NULL) {
        return;
    }


    uint8_t fsn = 0;
    uint8_t pdu_id = 0;


    if (transfer_frame->type == FRAME_FRAGMENTED) {
        fsn = transfer_frame->data.fragmented.pdu_header.FSN;
        pdu_id = transfer_frame->data.fragmented.pdu_header.PDU_ID;
    } else {
        fsn = transfer_frame->data.unfragmented.header.FSN;
        pdu_id = transfer_frame->data.unfragmented.header.PDU_ID;
    }


    // FARM-P only processes user-data frames for sequence control.
    if (pdu_id != PDU_DATA) {
        return;
    }


    if (fsn == farmp->rcv_rcvNxt) {
        // Expected frame received: advance V(R).
        farmp->rcv_rcvNxt++;
        farmp->rcv_retx = false;
    } else {
        // Sequence gap or unexpected frame: request retransmission.
        farmp->rcv_retx = true;
    }
}




/*Recibe un transfer frame de datos.
Actualiza el estado FARM-P.
Crea un P-frame con PLCW.
Devuelve ese P-frame listo para enviar.*/


bool farmp_process_frame_and_create_plcw(
    FarmPContext *farmp,
    const SDUFrame *received_frame,
    uint8_t expedited_frame_counter,
    uint8_t pcid,
    uint8_t PortID,
    uint16_t SC_ID,
    uint8_t SD_ID,
    SDUFrame *out_plcw_pframe
)
{
    if (farmp == NULL || received_frame == NULL || out_plcw_pframe == NULL) {
        return false;
    }


    farmp_process_transfer_frame(farmp, received_frame);


    *out_plcw_pframe = dataservice_create_plcw_pframe(
        farmp->rcv_rcvNxt,          // Report Value
        expedited_frame_counter,
        pcid,
        farmp->rcv_retx,            // Retransmit Flag
        PortID,
        SC_ID,
        SD_ID
    );


    return true;
}
void farmp_init(FarmPContext *farmp)
{
    if (farmp == NULL) {
        return;
    }


    farmp->rcv_rcvNxt = 0;
    farmp->rcv_retx = false;
    farmp->receiver_ready = true;
}








/*
Extrae el Report_Value del P-frame/PLCW recibido.
Actualiza el estado FOP-P con ese ACK (PLCW).
Borra de la SentFrameQueue los frames ya ACKeados.
Rellena los huecos de ventana con nuevos frames del IOBuffer.
Si no quedan frames pendientes, para el timer.
*/




bool fop_process_plcw(
    FopPContext *fop,
    SentFrameQueue *queue,
    IOBuffer *buffer,
    const SDUFrame *pframe
)
{
    if (fop == NULL || queue == NULL || buffer == NULL || pframe == NULL) {
        return false;
    }


    uint8_t report_value = 0;


    if (!dataservice_get_report_value(pframe, &report_value)) {
        return false;
    }


    fop->snd_prevRcvNxt = fop->snd_rcvNxt;
    fop->snd_rcvNxt = report_value;


    sent_frame_queue_remove_acked_fsn(queue, report_value);
    fill_sent_frame_queue_from_io_buffer(buffer, queue, fop);


    if (queue->count == 0) {
        dataservice_timer_stop(&fop->timer);
    }


    return true;
}





