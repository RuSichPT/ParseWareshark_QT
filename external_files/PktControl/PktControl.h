/*
 * PktControl.h
 *
 *  Created on: 22 июн. 2021 г.
 *      Author: Токарев Павел
 */

#ifndef PKT_CONTROL_H_
#define PKT_CONTROL_H_

#include "external_files/MAC/Inc/MAC_Virt_Channel.h"//Chain_buffer_elem_t
#include "external_files/MAC/Inc/MAC_pkts_format.h"//Pkt_Hdr, Pkt_Beacon, Pkt_type_Hdr, Valid_flag_t
#include "external_files/Routing/Rout_control_pkt.h"//Bool_t

#include "external_files/pbuffs.h" //QUEUE_TRUE_SZ

#define MAX_REPEAT_SERV_MSG                 (9) // максимальное кол-во в очереди повторяемых пакетов
#define REPEAT_SERV_MSG_QUEUE_SZ            (QUEUE_TRUE_SZ(MAX_REPEAT_SERV_MSG)) // размер очереди на отправку


#define MAX_BUFFERS_IN_CHAIN_CYCLE_BUF      2

#define MAX_MISSED_PKT                      (10) // максимальное кол-во хранимых номеров пропущенных пакетов
#define MAX_GROUP_CYCLE_WAIT_PKT            (2)  // максимальное кол-во групповых циклов ожидания пакетов

typedef enum {
	NONE_MODE		,
    WORKING         ,
    SYNCHRONIZATION ,
} Counter_mode_t;

#pragma pack(push, 1)

typedef struct {
    PKT_LEN_TYPE    pkt_len;
    uint8_t         pkt_number;
    Valid_flag_t    is_valid;
    uint8_t         was_sent         :1; // Флажок, говорящий о том, что пакет был повторно отправлен
    uint8_t         data[];
} Pkt_hdr_buf_t;

#pragma pack(pop)

typedef struct {
    Chain_buffer_elem_t*    first_chain_buf;
    Chain_buffer_elem_t*    cur_chain_buf;
    U8_T                    num_bufs_in_cyclebuf;
} Chain_cycle_buf_t;

typedef struct {
    uint8_t         pkt_number;
    Valid_flag_t    is_valid;
} Missed_pkt_t;

typedef struct {
    uint8_t         last_heard_pkt_number;              // порядковый номер последнего услышанного пакета(широковещательных), до первого пропуска
    uint8_t         current_pkt_number;                 // порядковый номер последнего услышанного пакета(широковещательных). Текущий номер в сети
    uint8_t         num_group_cycle_wait_pkt;        	// кол-во групповых циклов в ожидании пропущенного пакета
    Counter_mode_t  modes;                              // режимы работы счетчиков, 1 - синхронизация, 0 - штатная работа
    Missed_pkt_t    missed_pkts[MAX_MISSED_PKT];        // Номера пропущенных пакетов
} Counter_t;

extern Counter_t counters[MAX_RADIO_CONNECTIONS];

#if !defined (__OMNET__)        // OMNET Compiler
//!!!CONVERTER_START_FIELD_->

void init_pkt_control();

void init_chain_cycle_buf(Chain_cycle_buf_t *chain_cycb);

U8_T push_data_in_chain_cycle_buff(Chain_cycle_buf_t *chain_cycb, const Pkt_Hdr *PKTdata, const Pkt_Service *serv_pkt, uint16_t len);

void check_pkt_on_buffering(const Chain_buffer_elem_t *slot_buffer);

Pkt_hdr_buf_t* get_pkt_from_cycle_buff(U8_T pkt_number);

void find_and_delete_pkt_in_cycle_buff(const Pkt_Service *serviceData, PKT_ADDR_TYPE src_addr);

Bool_t search_index_in_queue(U8_T index, const indexQueue_TypeDef *queue, U8_T size_queue);

void counter_handler(const Pkt_Hdr *PKTdata, const Pkt_Beacon *beacon, U8_T RX_BI);

void repeating_station_counter_handler(const Pkt_Hdr *PKTdata, const Pkt_Beacon *beacon);

void losing_station_counter_handler(Counter_t *rx_counter, uint8_t last_heard_pkt_number);

Bool_t is_null_buf(const Pkt_hdr_buf_t *buf);

Bool_t has_missed_pkt(const Counter_t *cntr);

void set_missed_pkt(Counter_t *cntr, uint8_t pkt_number);

Missed_pkt_t* get_missed_pkt(Counter_t *cntr, uint8_t pkt_number);

void delete_missed_pkt(Missed_pkt_t *missed_pkt);

void delete_all_missed_pkts(Counter_t *cntr);

void set_synchronization_mode(Counter_t *cntr);

void update_current_pkt_number(uint8_t pkt_number, Counter_t *counter);

void update_last_heard_pkt_number(uint8_t pkt_number, Counter_t *counter);

U8_T update_packet_number(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr);

//!!!CONVERTER_END_FIELD_<-
#endif

#endif /* PKT_CONTROL_H_ */
