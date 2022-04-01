/*
 * PktControl.cс
 *
 *  Created on: 22 июн. 2021 г.
 *      Author: Токарев Павел
 */

#if defined (__OMNET__)        // OMNET Compiler

#include "MacLayer.h"

namespace inet
{
#else

#include "MAC/Inc/MAC_Timeline.h" // MAX_RADIO_CONNECTIONS
#include "MAC/Inc/MAC_TX_BuildPkts.h" // SLOT_BUFFER_OFFSET
#include "MAC/Inc/MAC.h" // mac, find_Addr_pos
#include "MAC/Inc/MAC_VCh_Reserving.h"//chain_buffer_list
#include "PktControl/PktControl.h"
#include "Routing/Rout_control_pkt.h"// compare_num8
#include "tools.h"//Wiki_Crc16
#include <string.h>//memset

//!!!CONVERTER_START_FIELD_->

Counter_t counters[MAX_RADIO_CONNECTIONS];                                                  // Счетчики пакетов на каждого соседнего абонента

Chain_cycle_buf_t msg_cycle_buf;                                                            // Место для хранения повторных пакетов

uint8_t repeat_service_msg_queue[QUEUE_BUF_SZ(REPEAT_SERV_MSG_QUEUE_SZ)];                   // Очередь для отправки повторных пакетов

//!!!CONVERTER_END_FIELD_<-
#endif

void F_CLASS init_pkt_control()
{
	memset(counters, 0, sizeof(counters));
	memset((uint8_t *)&repeat_service_msg_queue, 0, sizeof(repeat_service_msg_queue));

	// Инициализация msg_cycle_buf должна идти позже инициализации chain_buffer_list
	init_chain_cycle_buf(&msg_cycle_buf);
}

void F_CLASS init_chain_cycle_buf(Chain_cycle_buf_t *chain_cycb)
{
	Chain_buffer_elem_t *chain_buffer;
	U8_T chain_pbuf_index;

	chain_buffer = create_chain_buffer(FOR_REPEAT_PKT);
	if (chain_buffer == NULL)
	{
		return;
	}

	chain_pbuf_index = chain_buffer->pbuf_index;

	chain_cycb->first_chain_buf = chain_buffer;
	chain_cycb->cur_chain_buf = chain_buffer;
	chain_cycb->num_bufs_in_cyclebuf = 1;

	memset(pbuffs_to_RXTX[chain_pbuf_index].buff, 0, sizeof(Pkt_hdr_buf_t));
	chain_buffer->start_len = BUFF_SIZE;
}

U8_T F_CLASS push_data_in_chain_cycle_buff(Chain_cycle_buf_t *chain_cycb, const Pkt_Hdr *PKTdata, const Pkt_Service *serv_pkt, uint16_t len)
{
	Chain_buffer_elem_t *chain_buffer;
	Pkt_hdr_buf_t *pkt_hdr;
	Pkt_Hdr *new_PKTdata;

	U8_T chain_pbuf_index;

	chain_buffer = chain_cycb->cur_chain_buf;

	if ((chain_buffer->end_len + len + sizeof(Pkt_hdr_buf_t) + sizeof(Pkt_hdr_buf_t)) < BUFF_SIZE)
	{
		if (chain_buffer->start_len < BUFF_SIZE)
		{
			if ((chain_buffer->end_len + len + sizeof(Pkt_hdr_buf_t) + sizeof(Pkt_hdr_buf_t)) >= chain_buffer->start_len)
			{
				//удаляем пакеты из начала цепочки, занимая их место
				do
				{
					pkt_hdr = (Pkt_hdr_buf_t*)&pbuffs_to_RXTX[chain_buffer->pbuf_index].buff[chain_buffer->start_len];
					if (pkt_hdr->pkt_len != 0)
					{
						chain_buffer->start_len = chain_buffer->start_len + pkt_hdr->pkt_len + sizeof(Pkt_hdr_buf_t);
					}
					else
					{
						chain_buffer->start_len = BUFF_SIZE;
					}
				}
				while (((chain_buffer->end_len + len + sizeof(Pkt_hdr_buf_t) + sizeof(Pkt_hdr_buf_t)) > (chain_buffer->start_len)));
			}
		}

		pkt_hdr = (Pkt_hdr_buf_t*)&pbuffs_to_RXTX[chain_buffer->pbuf_index].buff[chain_buffer->end_len];
		pkt_hdr->pkt_len = len;
		pkt_hdr->was_sent = 0;
		pkt_hdr->is_valid = VALID;
		pkt_hdr->pkt_number = serv_pkt->pkt_number;

		MEMCPY(pkt_hdr->data, PKTdata, PKT_HDR_LEN);
		new_PKTdata = (Pkt_Hdr*)(pkt_hdr->data);
		new_PKTdata->PktLen = len - PKT_HDR_LEN;
		MEMCPY(pkt_hdr->data + PKT_HDR_LEN, serv_pkt, len - PKT_HDR_LEN);
		new_PKTdata->Data_LenToNext = 0;

		new_PKTdata->crc = 0;
		new_PKTdata->crc = Wiki_Crc16((uint8_t*) new_PKTdata, new_PKTdata->PktLen + PKT_HDR_LEN);

		chain_buffer->end_len = chain_buffer->end_len + len + sizeof(Pkt_hdr_buf_t);
		pbuffs_to_RXTX[chain_buffer->pbuf_index].len = chain_buffer->end_len;

		pkt_hdr = (Pkt_hdr_buf_t*)&pbuffs_to_RXTX[chain_buffer->pbuf_index].buff[chain_buffer->end_len];
		pkt_hdr->pkt_len = 0;
	}
	else if (chain_cycb->num_bufs_in_cyclebuf < MAX_BUFFERS_IN_CHAIN_CYCLE_BUF)
	{
		chain_buffer->next = create_chain_buffer(FOR_REPEAT_PKT);
		if (chain_buffer->next == NULL)
		{
			return 0;
		}

		chain_buffer = chain_buffer->next;

		chain_pbuf_index = chain_buffer->pbuf_index;
		pbuffs_to_RXTX[chain_pbuf_index].len = len;
		memset(pbuffs_to_RXTX[chain_pbuf_index].buff, 0, sizeof(Pkt_hdr_buf_t));

		chain_buffer->start_len = BUFF_SIZE;

		chain_cycb->cur_chain_buf = chain_buffer;
		chain_cycb->num_bufs_in_cyclebuf ++;

		if (chain_cycb->num_bufs_in_cyclebuf == MAX_BUFFERS_IN_CHAIN_CYCLE_BUF)
		{
			chain_buffer->next = chain_cycb->first_chain_buf;
		}

		push_data_in_chain_cycle_buff(chain_cycb, PKTdata, serv_pkt, len);
	}
	else
	{
		//место кончилось, переходим к следующему буферу в цепочке
		chain_cycb->cur_chain_buf = chain_cycb->cur_chain_buf->next;
		chain_cycb->cur_chain_buf->start_len = 0;
		chain_cycb->cur_chain_buf->end_len = 0;

		push_data_in_chain_cycle_buff(chain_cycb, PKTdata, serv_pkt, len);
	}

	return 1;
}

void F_CLASS check_pkt_on_buffering(const Chain_buffer_elem_t *slot_buffer)
{
	U8_T pbuf_index;
	Pkt_hdr_buf_t *pkt_hdr;
	Pkt_Hdr *PKTdata;
	Pkt_Service *serviceData;
	uint16_t LenToNext_field;
	Pkt_type_Hdr *type_Hdr;
	uint16_t len;

	pbuf_index = slot_buffer->pbuf_index;

	PKTdata = (Pkt_Hdr *)&pbuffs_to_RXTX[pbuf_index].buff[SLOT_BUFFER_OFFSET];

	if (PKTdata->DestAddr == SOFT_ADDR_BRDCST)
	{
		LenToNext_field = 0;
		type_Hdr = (Pkt_type_Hdr*)&(PKTdata->Data_type);

		do //Piggy-back по PKTdata->pData_LenToNext - по разным типам пакетов внутри одного пакета с хедером
		{
			LenToNext_field = type_Hdr->LenToNext;

			switch (type_Hdr->Type)
			{
				case PKT_TYPE_RTS:
				case PKT_TYPE_CTS:
				case PKT_TYPE_ACK:
				case PKT_TYPE_NACK:
				case PKT_TYPE_DATA_MAP:
				case PKT_TYPE_DATA_MAP_REQUEST:
				serviceData = (Pkt_Service*)type_Hdr;

				pkt_hdr = get_pkt_from_cycle_buff(serviceData->pkt_number);
				if (pkt_hdr == NULL) // нет такого пакета в буфере, добавляем
				{
					find_and_delete_pkt_in_cycle_buff(serviceData, PKTdata->SrcAddr);

					if (LenToNext_field != 0)
					{
						len = LenToNext_field + sizeof(Pkt_Hdr);
					}
					else
					{
						len = PKTdata->PktLen - (((uint8_t*)type_Hdr) - ((uint8_t*)&(PKTdata->Data_type))) + PKT_HDR_LEN;
					}
					push_data_in_chain_cycle_buff(&msg_cycle_buf, PKTdata, serviceData, len);
				}

				break;
				default:
				break;
			}

			type_Hdr = (Pkt_type_Hdr*) ( ((uint8_t*)(&(type_Hdr->LenToNext))) + sizeof(PKT_LENTONEXT_TYPE) + LenToNext_field );
		}
		while (LenToNext_field != 0);
	}
}

Pkt_hdr_buf_t* F_CLASS get_pkt_from_cycle_buff(U8_T pkt_number)
{
	Chain_buffer_elem_t *chain_buffer;
	Pkt_hdr_buf_t *pkt_hdr;
	uint16_t read_idx;

	// Берем первый элемент
	chain_buffer = msg_cycle_buf.first_chain_buf;
	read_idx = 0;
	do
	{
		pkt_hdr = (Pkt_hdr_buf_t*)&pbuffs_to_RXTX[chain_buffer->pbuf_index].buff[read_idx];
		// Метка конца буфера
		if (pkt_hdr->pkt_len == 0)
		{
			if ((chain_buffer->start_len < BUFF_SIZE) && (read_idx < chain_buffer->start_len))
			{
				read_idx = chain_buffer->start_len;
			}
			else
			{
				if ((chain_buffer->next != NULL) && (chain_buffer->next != msg_cycle_buf.first_chain_buf))
				{
					chain_buffer = chain_buffer->next;
					read_idx = 0;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			if (pkt_hdr->pkt_number == pkt_number)
			{
				return pkt_hdr;
			}

			read_idx = read_idx + pkt_hdr->pkt_len + sizeof(Pkt_hdr_buf_t);
		}
	}
	while (1);

	return NULL;
}

void F_CLASS find_and_delete_pkt_in_cycle_buff(const Pkt_Service *serviceData, PKT_ADDR_TYPE src_addr)
{
	Chain_buffer_elem_t *chain_buffer;
	Pkt_hdr_buf_t *pkt_hdr;
	Pkt_Hdr *PKTdata;
	Pkt_Service *cur_serviceData;
	uint16_t read_idx;

	// Берем первый элемент
	chain_buffer = msg_cycle_buf.first_chain_buf;
	read_idx = 0;
	do
	{
		pkt_hdr = (Pkt_hdr_buf_t*)&pbuffs_to_RXTX[chain_buffer->pbuf_index].buff[read_idx];
		// Метка конца буфера
		if (pkt_hdr->pkt_len == 0)
		{
			if ((chain_buffer->start_len < BUFF_SIZE) && (read_idx < chain_buffer->start_len))
			{
				read_idx = chain_buffer->start_len;
			}
			else
			{
				if ((chain_buffer->next != NULL) && (chain_buffer->next != msg_cycle_buf.first_chain_buf))
				{
					chain_buffer = chain_buffer->next;
					read_idx = 0;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			if (pkt_hdr->is_valid)
			{
				PKTdata = (Pkt_Hdr *)(pkt_hdr->data);
				cur_serviceData = (Pkt_Service*)&(PKTdata->Data_type);

				switch(serviceData->Type)
				{
					case PKT_TYPE_NACK:
					if ((serviceData->closing_reason == REASON_TRAFF_END) || (serviceData->closing_reason == REASON_TRAFF_END_REP))
					{
						if ( (PKTdata->Data_type == PKT_TYPE_DATA_MAP)
								&& (cur_serviceData->virt_ch_num == serviceData->virt_ch_num)
								&& (cur_serviceData->src_addr == serviceData->src_addr)
								&& (cur_serviceData->sec_addr == serviceData->sec_addr)
								&& (cur_serviceData->res_entry[0].direction == serviceData->res_entry[0].direction)
								&& (PKTdata->SrcAddr == src_addr)
						)
						{
							pkt_hdr->is_valid = INVALID;
						}
					}
					break;
					default:
					if ( (PKTdata->Data_type == serviceData->Type)
							&& (cur_serviceData->virt_ch_num == serviceData->virt_ch_num)
							&& (cur_serviceData->src_addr == serviceData->src_addr)
							&& (cur_serviceData->sec_addr == serviceData->sec_addr)
							&& (cur_serviceData->res_entry[0].direction == serviceData->res_entry[0].direction)
							&& (PKTdata->SrcAddr == src_addr)
					)
					{
						pkt_hdr->is_valid = INVALID;
					}
					break;
				}
			}
			read_idx = read_idx + pkt_hdr->pkt_len + sizeof(Pkt_hdr_buf_t);
		}
	}
	while (1);

	return;
}

Bool_t F_CLASS search_index_in_queue(U8_T index, const indexQueue_TypeDef *queue, U8_T size_queue)
{
	QUEUE_SIZE_T read_idx, queue_size;

	read_idx = queue->read_idx;
	queue_size = Check_QueueSize(queue, size_queue);

	while (queue_size > 0)
	{
		if (queue->IndexTable[read_idx] == index)
		{
			return TRUE_C; // нашли
		}

		read_idx = (read_idx + 1) % size_queue;
		queue_size --;
	}

	return FALSE_C; // не нашли
}

void F_CLASS counter_handler(const Pkt_Hdr *PKTdata, const Pkt_Beacon *beacon, U8_T RX_BI)
{
	Counter_t *rx_counter;

	rx_counter = &counters[RX_BI];

	// Для абонента который повторяет пакеты
	repeating_station_counter_handler(PKTdata, beacon);

	// Для абонента который пропускает пакеты
	losing_station_counter_handler(rx_counter, beacon->last_heard_pkt_number[RX_BI]);
}

void F_CLASS repeating_station_counter_handler(const Pkt_Hdr *PKTdata, const Pkt_Beacon *beacon)
{

	Counter_t *own_counter;
	Pkt_hdr_buf_t *pkt_hdr_buf;
	Pkt_Hdr *pkt_hdr;
	uint8_t pkt_number, diff_num_pkt, j, recieved_pkt_number;
	PKT_LEN_TYPE total_len, max_pkt_len;

	recieved_pkt_number = beacon->last_heard_pkt_number[mac.CurrentBeaconInterval];
	own_counter = &counters[mac.CurrentBeaconInterval];

	// Проверяем вид связи на абонента
	if ( (beacon->AS[mac.CurrentBeaconInterval].addr == mac.addr)
			&& ((get_VSV((uint8_t *)beacon->VSV, mac.CurrentBeaconInterval) == VSV_RSV)
					|| (get_VSV((uint8_t *)beacon->VSV, mac.CurrentBeaconInterval) == VSV_RPSV))
	)
	{
		// Смотрим все ли широковещательные служебные пакеты были услышаны
		if (compare_num8(own_counter->last_heard_pkt_number, recieved_pkt_number))// знаком (>) исключаем вызов этой функции у абонента теряющего пакеты (у него <)
		{
#if (defined (__OMNET__)) && (DEBUG_WITH_ETH_SEND)
			if (Debug_commands.debug_Proc_PKT_ERR != 0)
			{
				Debug_Str_Send(ERROR_String, user_sprintf_int(ERROR_String, "Missing pkt: pkt_number=% Src=% Who missed=%",
								recieved_pkt_number + 1 , mac.addr, PKTdata->SrcAddr), EN_DEBUG_VCH_CTRL);
			}
#endif
			// Макс длина полезных данных, кот. можно уместить в служебном слоте
			max_pkt_len = GetLen_fromRate(MIN_SIZE_DATA_BLOCK, MIN_SPEED, UCOS_KSS_TYPE_NORM) - PKT_HDR_LEN;

			diff_num_pkt = own_counter->last_heard_pkt_number - recieved_pkt_number;

			for (j = 0; j < diff_num_pkt; j++)
			{
				// Номер пакета для повторной отправки
				pkt_number = recieved_pkt_number + 1 + j;
				// Ищем пакет
				pkt_hdr_buf = get_pkt_from_cycle_buff(pkt_number);

				if (!is_null_buf(pkt_hdr_buf))
				{
					if (total_len <= max_pkt_len)
					{
						pkt_hdr = (Pkt_Hdr *)(pkt_hdr_buf->data);
						total_len += pkt_hdr->PktLen;
					}
					else
					{
						return;
					}

					// Ищем в очереди пакет, если не находим добавляем в очередь
					if (!search_index_in_queue(pkt_number, (indexQueue_TypeDef *)repeat_service_msg_queue, REPEAT_SERV_MSG_QUEUE_SZ))
					{
						if (!push_pDataInQueue(pkt_number, (indexQueue_TypeDef *)repeat_service_msg_queue, REPEAT_SERV_MSG_QUEUE_SZ))
						{
#if (DEBUG_ASM_BKPT)
							ERROR_DEBUG_BKPT; // очередь на отправку повторных пакетов заполнилась
#endif
						}
					}
				}
				else
				{
					continue;
				}
			}
		}
	}
}

void F_CLASS losing_station_counter_handler(Counter_t *rx_counter, uint8_t last_heard_pkt_number)
{
	uint8_t pkt_number;

	if (rx_counter->modes == WORKING)
	{
		// Ожидаем пакет слишком долго
		if (rx_counter->num_group_cycle_wait_pkt > MAX_GROUP_CYCLE_WAIT_PKT)
		{
#if (defined (__OMNET__)) && (DEBUG_WITH_ETH_SEND)
			if (Debug_commands.debug_Proc_PKT_ERR != 0)
			{
				Debug_Str_Send(ERROR_String, user_sprintf_int(ERROR_String, "Long Wait"), EN_DEBUG_VCH_CTRL);
			}
#endif
			pkt_number = rx_counter->last_heard_pkt_number;
			pkt_number++;
			update_last_heard_pkt_number(pkt_number, rx_counter);
		}
	}
	else if (rx_counter->modes == SYNCHRONIZATION)
	{
		// Синхронизация счетчика пакета для конкретного абонента
		rx_counter->last_heard_pkt_number = last_heard_pkt_number;
		rx_counter->current_pkt_number = last_heard_pkt_number;
		// Входим в штатный режим работы счетчиков
		rx_counter->modes = WORKING;
#if (defined (__OMNET__)) && (DEBUG_WITH_ETH_SEND)
		if (Debug_commands.debug_Proc_PKT_ERR != 0)
		{
			Debug_Str_Send(ERROR_String, user_sprintf_int(ERROR_String, "SYNCHRONIZATION"), EN_DEBUG_VCH_CTRL);
		}
#endif
	}
}

Bool_t F_CLASS has_missed_pkt(const Counter_t *cntr)
{
	// Проверяем наличие пропущенных пакетов
	U8_T i;

	for (i = 0; i < MAX_MISSED_PKT; i++)
	{
		if (cntr->missed_pkts[i].is_valid == VALID)
		{
			return TRUE_C;
		}
	}

	return FALSE_C;
}

void F_CLASS set_missed_pkt(Counter_t *cntr, uint8_t pkt_number)
{
	U8_T i;

	for (i = 0; i < MAX_MISSED_PKT; i++)
	{
		if (cntr->missed_pkts[i].is_valid == INVALID)
		{
			cntr->missed_pkts[i].is_valid = VALID;
			cntr->missed_pkts[i].pkt_number = pkt_number;
			return;
		}
	}

	if (i == MAX_MISSED_PKT)
	{
#if (DEBUG_ASM_BKPT)
		ERROR_DEBUG_BKPT; // нет места в списке
#endif
		set_synchronization_mode(cntr);
		return;
	}
}

Missed_pkt_t* F_CLASS get_missed_pkt(Counter_t *cntr, uint8_t pkt_number)
{
	U8_T i;

	for (i = 0; i < MAX_MISSED_PKT; i++)
	{
		if ( (cntr->missed_pkts[i].is_valid == VALID) && (cntr->missed_pkts[i].pkt_number == pkt_number) )
		{
			return &cntr->missed_pkts[i]; // нашли
		}
	}

	return NULL; // не нашли
}

void F_CLASS update_current_pkt_number(uint8_t pkt_number, Counter_t *counter)
{
	U8_T i, amount_missed_pkt;

	// Находим пропущенные пакеты
	amount_missed_pkt = pkt_number - (uint8_t)(counter->current_pkt_number + 1);

	// Есть пропуск
	if (amount_missed_pkt != 0)
	{
		for (i = 0; i < amount_missed_pkt; i++)
		{
			// добавляем пакет в пропущенные
			set_missed_pkt(counter, (uint8_t)(counter->current_pkt_number + 1 + i) );
		}
	}

	// Обновляем текущий счетчик
	counter->current_pkt_number = pkt_number;

	// Если нет пропущенных пакетов
	if (!has_missed_pkt(counter))
	{
		// То последний услышанный равен текущему
		counter->last_heard_pkt_number = counter->current_pkt_number;
	}
}

void F_CLASS update_last_heard_pkt_number(uint8_t pkt_number, Counter_t *counter)
{
	Missed_pkt_t *missed_pkt;

	// Сбрасываем счетчик ожидания при каждом приеме повторного пакета
	counter->num_group_cycle_wait_pkt = 0;
	counter->last_heard_pkt_number++;

	// Ищем пакет в пропущенных
	missed_pkt = get_missed_pkt(counter, pkt_number);
	if (missed_pkt != NULL)// нашли
	{
		delete_missed_pkt(missed_pkt);
	}
#if (DEBUG_ASM_BKPT)
	else
	{
		ERROR_DEBUG_BKPT; // нет пропущенного пакета в ветке приема пропущенных
	}
#endif
	// Пытаемся догнать текущий счетчик
	while (counter->current_pkt_number != counter->last_heard_pkt_number)
	{
		// Ищем следующий номер пакета в пропущенных
		missed_pkt = get_missed_pkt(counter, (uint8_t)(counter->last_heard_pkt_number + 1));
		if (missed_pkt != NULL)// нашли
		{
			break;
		}
		counter->last_heard_pkt_number++;
	}
}

U8_T F_CLASS update_packet_number(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr)
{
	U8_T beacon_interval;

	Pkt_Service *service_pkt;
	Pkt_control_number *control_pkt;
	Counter_t *counter;

	uint8_t pkt_number;

	if (type_Hdr->Type == PKT_TYPE_CONTROL_NUMBER)
	{
		control_pkt = (Pkt_control_number *)type_Hdr;
		pkt_number = control_pkt->pkt_number;
	}
	else
	{
		service_pkt = (Pkt_Service *)type_Hdr;
		pkt_number = service_pkt->pkt_number;
	}

	beacon_interval = find_Addr_pos(PKTdata->SrcAddr);
	if (beacon_interval == 0)
	{
		// Приняли пакет от абонента которого нет в адресной таблице
		// Проверяем адресную книгу
		beacon_interval = Find_In_AddressBook(PKTdata->SrcAddr);
		if (beacon_interval == 0)
		{
			return 1;
		}
		beacon_interval --;
		counter = &counters[beacon_interval];
		// Обновляем все счетчики
		counter->current_pkt_number = pkt_number;
		counter->last_heard_pkt_number = counter->current_pkt_number;
		return 1;
	}
	beacon_interval --;

	counter = &counters[beacon_interval];

	if (counter->modes == WORKING)
	{
		// Проверяем текущий счетчик на наличие потери пакета
		if ( compare_num8(pkt_number, counter->current_pkt_number) )// Если номер пакета больше
		{
			update_current_pkt_number(pkt_number, counter);
		}
		// Прием повторных пакетов
		else if ( pkt_number == ( (uint8_t)(counter->last_heard_pkt_number + 1) ) )
		{
			update_last_heard_pkt_number(pkt_number, counter);
		}
		else
		{
			return 0; // ( pkt_number < counter->last_heard_pkt_number + 1 )
		}
	}

	return 1;
}

inline Bool_t F_CLASS is_null_buf(const Pkt_hdr_buf_t *buf)
{
	if (buf == NULL)
	{
#if (DEBUG_ASM_BKPT)
		ERROR_DEBUG_BKPT; // нет такого пакета в буфере
#endif
		return TRUE_C;
	}

	return FALSE_C;
}

inline void F_CLASS delete_missed_pkt(Missed_pkt_t *missed_pkt)
{
    missed_pkt->is_valid = INVALID;
    missed_pkt->pkt_number = 0;
}

inline void F_CLASS delete_all_missed_pkts(Counter_t *cntr)
{
    memset((uint8_t*)cntr->missed_pkts, 0, sizeof(Missed_pkt_t)*MAX_MISSED_PKT);
}

inline void F_CLASS set_synchronization_mode(Counter_t *cntr)
{
	cntr->modes = SYNCHRONIZATION;
	cntr->num_group_cycle_wait_pkt = 0;
	delete_all_missed_pkts(cntr);
}


#if defined (__OMNET__)        // OMNET Compiler
}
 // namespace inet
#endif

