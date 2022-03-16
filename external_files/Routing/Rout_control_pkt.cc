/*
 * Rout_control_pkt.cc

 *
 *  Created on: 18 дек. 2020 г.
 *      Author: Токарев Павел
 */
#if defined (__OMNET__)        // OMNET Compiler

#include "MacLayer.h"

namespace inet
{
#else

#include "MAC/Inc/ModemModel.h"//Modem_SendCommand_FSET
#include "MAC/Inc/MAC.h"
#include "MAC/Inc/ModemProtocol.h"
#include "MAC/Inc/TaskManager.h"
#include "MAC/Inc/MAC_RXTX_TasksProc.h"
#include "external_files/MAC/Inc/MAC_Timeline.h"
#include "external_files/MAC/Inc/MAC_pkts_format.h"
#include "MAC/Inc/MAC_TX_BuildPkts.h"
#include "external_files/Routing/Rout_control_pkt.h"
#include "MAC/Inc/MAC_TasksGen.h"//для TASK_DIR_RX TASK_DIR_TX
#include "MAC/Inc/MAC_Connect_to_Net.h"
#include "MAC/Inc/ModemFHSS.h"//Calc_FRQ_forModemCommandWord_NORM
#include "external_files/MAC/Inc/MAC_Timers.h"//GetINR()
#include "MAC/Inc/MAC_Status_Info_Gen.h"//INFO_TRAFF_TX
#include "MAC/Inc/MAC_Data_Queues.h"
#include "tools.h"//Wiki_Crc16
#include <string.h>//memset


//!!!CONVERTER_START_FIELD_->

Net_addr_t net_addr_table[MAX_RADIO_CONNECTIONS];                   // таблица соответствия mac, ip адресов и наших сетевых адресов в сети

Route_row_t routing_table[MAX_SIZE_TABLE_ROUTE];                    // таблица маршрутизации

Black_list_elem_t black_list[MAX_SIZE_LIST_ROUTE];                  // блэк лист REQ запросов

Route_elem_t serv_msg_route_list[MAX_SIZE_LIST_ROUTE];              // Хранилище пакетов на отправку

uint8_t serv_msg_route_queue[QUEUE_BUF_SZ(SERV_MSG_ROUT_QUEUE_SZ)]; // Очередь на отправку

//!!!CONVERTER_END_FIELD_<-
#endif

void F_CLASS init_routing()
{
	memset(net_addr_table, EMPTY, sizeof(net_addr_table));
	memset(routing_table, EMPTY, sizeof(routing_table));
	memset((uint8_t *)&black_list, EMPTY, sizeof(black_list));
	memset((uint8_t *)&serv_msg_route_list, EMPTY, sizeof(serv_msg_route_list));
	memset((uint8_t *)&serv_msg_route_queue, 0, sizeof(serv_msg_route_queue));
}

void F_CLASS handle_routing()
{
	if (mac.state != CONNECTED)
	{
		return;
	}

	tick_route_lifetime();

	check_deletion_routes();

	search_routes();

	if (mac.mode == ETHERNET)
	{
		search_mac_ip();
	}
}

void F_CLASS tick_route_lifetime()
{
	U8_T i;
	Route_row_t *route;

	for (i = 0; i < MAX_SIZE_TABLE_ROUTE; i++)
	{
		route = &routing_table[i];
		if (route->state != INACTIVE && route->state != WAIT_DELETE)
		{
			route->lifetime--;

			if (route->lifetime == 0)
			{
				set_route_state(route, INACTIVE);
			}
		}
	}
}

void F_CLASS check_deletion_routes()
{
    U8_T i, j;
    uint8_t num_break_route;
    Route_row_t *route;
    Unreachable_node_t unreachable_nodes[MAX_RADIO_CONNECTIONS];
    Route_elem_t *route_elem;
    Flags_t flags;

    for (i = 0; i < MAX_SIZE_TABLE_ROUTE; i++)
    {
        route = &routing_table[i];

        if (route->state == WAIT_DELETE)
        {
            memset(&flags, 0, sizeof(Flags_t));

            route->seq_num++;

            num_break_route = get_unreachable_nodes(unreachable_nodes, route);

            route_elem = create_route_elem(NOT_USED, NOT_USED,
                                            mac.addr, NOT_USED, SOFT_ADDR_BRDCST,
                                            NOT_USED, NOT_USED, flags,
                                            SEND_RERR, num_break_route, unreachable_nodes);
            if (route_elem == NULL)
            {
                continue;
            }

            send_route_elem(route_elem);

            for (j = 0; j < num_break_route; j++)
            {
                route = get_route(unreachable_nodes[j].sta_addr);

                if (route == NULL)
                {
#if (DEBUG_ASM_BKPT)
                    ERROR_DEBUG_BKPT; // нет маршрута
#endif
                    continue;
                }

                set_route_state(route, INACTIVE);
            }
        }
    }
}

void F_CLASS search_routes()
{
	U8_T i;
	Route_elem_t *route_elem;
	Route_row_t *route;
	Flags_t flags;

	for (i = 0; i < MAX_SIZE_TABLE_ROUTE; i++)
	{
	    route = &routing_table[i];

	    if (route->state == INACTIVE && route->sta_addr != EMPTY)
		{
            memset(&flags, 0, sizeof(Flags_t));

	    	add_black_list(mac.addr,++mac.rreq_id);// Буферизируем REQ, чтобы свои REQ отбрасывать на приеме

            set_route_state(route, WAIT_RREP);
            route->lifetime = TIME_WAIT_RREP_FRAMES;// время нахождения в состоянии WAIT_RREP

			flags.gratuitous_RREP = 1;
			if (route->valid_flag == INVALID)
			{
				flags.unknown_seq_num = 1;
			}

			route_elem = create_route_elem(route->sta_addr, route->seq_num,
			                                mac.addr, ++mac.seq_num, SOFT_ADDR_BRDCST,
			                                mac.rreq_id, 0, flags,
			                                SEND_RREQ, NOT_USED, NOT_USED);
			send_route_elem(route_elem);
		}
	}
}

void F_CLASS search_mac_ip()
{
	U8_T i;
	Route_row_t *route;
    Net_addr_t *as_net;
	Route_elem_t *route_elem;
	Flags_t flags;

	if ( (mac.ip_addr != 0) && (mac.cycle % TIME_WAIT_MAC_IP_REP_FRAMES == 0) )
	{
		for (i = 0; i < MAX_SIZE_TABLE_ROUTE; i++)
		{
			route = &routing_table[i];

			if (route->state == ACTIVE)
			{
				as_net = get_as_by_addr_in_net(route->sta_addr);

				if (as_net == NULL)
				{
					memset(&flags, 0, sizeof(Flags_t));

					route_elem = create_route_elem(route->sta_addr, NOT_USED,
													mac.addr, NOT_USED, route->next_hop_addr,
													NOT_USED, NOT_USED, flags,
													SEND_MAC_IP_REQ, NOT_USED, NOT_USED);
					send_route_elem(route_elem);
				}
			}
		}
	}
}

uint8_t F_CLASS get_unreachable_nodes(Unreachable_node_t *unreachable_nodes, const Route_row_t *unreachable_route)
{
    U8_T i, dest_count = 0;
    Route_row_t *route;

    // Добавляем абонента с WAIT_DELETE
    unreachable_nodes[dest_count].sta_addr = unreachable_route->sta_addr;
    unreachable_nodes[dest_count].seq_num = unreachable_route->seq_num;
    unreachable_nodes[dest_count].valid_flag = unreachable_route->valid_flag;

    dest_count++;

    // Ищем все остальные поврежденные маршруты, для которых абонент с WAIT_DELETE является шлюзом (next_hop_STA)
    for (i = 0; i < MAX_SIZE_TABLE_ROUTE; i++)
    {
        route = &routing_table[i];

        if (route->sta_addr == EMPTY)
        {
            continue;
        }

        if ( (route->next_hop_addr == unreachable_route->sta_addr) && (route->state == ACTIVE) )
        {

            unreachable_nodes[dest_count].sta_addr = route->sta_addr;

            if (route->valid_flag == VALID)
            {
                unreachable_nodes[dest_count].seq_num = ++route->seq_num;
                unreachable_nodes[dest_count].valid_flag = VALID;
            }
            else
            {
                unreachable_nodes[dest_count].valid_flag = INVALID;
            }

            dest_count++;
        }
    }

    return dest_count;
}

Route_row_t* F_CLASS create_route(uint16_t sta_addr, Valid_flag_t flag, uint32_t seq_num, uint8_t hop_count,
                                    uint16_t next_hop_addr, Route_state_t state, uint32_t lifetime)
{
	U8_T i;

	if (sta_addr == mac.addr)
	{
#if (DEBUG_ASM_BKPT)
        ERROR_DEBUG_BKPT;// создаем маршрут на себя
#endif
        return NULL;
	}

	for (i = 0; i < MAX_SIZE_TABLE_ROUTE; i++)
	{
		if (routing_table[i].sta_addr == EMPTY)
		{
		    routing_table[i].sta_addr = sta_addr;
		    routing_table[i].valid_flag = flag;
		    routing_table[i].seq_num = seq_num;
		    routing_table[i].hop_count = hop_count;
		    routing_table[i].next_hop_addr = next_hop_addr;
		    set_route_state(&routing_table[i], state);
		    routing_table[i].lifetime = lifetime;

		    return &routing_table[i];
		}
	}

#if (DEBUG_ASM_BKPT)
		ERROR_DEBUG_BKPT;// нет места в таблице маршрутизации routing_table
#endif

    return NULL;
}

void F_CLASS update_route(Route_row_t *route, uint32_t seq_num, Valid_flag_t valid_state, uint16_t next_hop_addr,
                            uint8_t hop_count, Route_state_t state, uint32_t lifetime)
{
    route->lifetime = lifetime;
    route->seq_num = seq_num;
    route->hop_count = hop_count;   // Note: already incremented by 1.
    route->next_hop_addr = next_hop_addr;
    route->valid_flag = valid_state;
    set_route_state(route, state);
}

void F_CLASS set_route_state(Route_row_t *route, Route_state_t state)
{
    route->state = state;
}

U8_T F_CLASS alloc_route_elem()
{
    U8_T i;

    for (i = 0; i < MAX_SIZE_LIST_ROUTE; i++)
    {
        if (serv_msg_route_list[i].state == NOT_STATE)
        {
            return i;
        }
    }

#if (DEBUG_ASM_BKPT)
    ERROR_DEBUG_BKPT;// нет места в списке
#endif

    return MAX_SIZE_LIST_ROUTE;

}

Route_elem_t* F_CLASS find_route_elem(const Route_elem_t *route_elem)
{
	U8_T i;

	for (i = 0; i < MAX_SIZE_LIST_ROUTE; i++)
	{
	    if (serv_msg_route_list[i].state == NOT_STATE) continue;

        if (route_elem->index_route_list == i) continue;

	    if (route_elem->state != serv_msg_route_list[i].state) continue;

        if (route_elem->dest_sta_addr != serv_msg_route_list[i].dest_sta_addr) continue;

        if (route_elem->orig_sta_addr != serv_msg_route_list[i].orig_sta_addr) continue;

        return &serv_msg_route_list[i];// нашли
	}
	return NULL; // не нашли
}

void F_CLASS send_route_elem(Route_elem_t *route_elem)
{
    Route_elem_t *route_elem_old;

    if (route_elem == NULL)
    {
        return;
    }

    if (route_elem->state == NOT_STATE)
    {
#if (DEBUG_ASM_BKPT)
        ERROR_DEBUG_BKPT; // нет состояния
#endif
        return;
    }

    // Ищем в списке route_elem, чтобы не накапливать
    route_elem_old = find_route_elem(route_elem);
    if (route_elem_old == NULL)
    {
        if (!push_pDataInQueue(route_elem->index_route_list, (indexQueue_TypeDef *)serv_msg_route_queue, SERV_MSG_ROUT_QUEUE_SZ))
        {
    #if (DEBUG_ASM_BKPT)
            ERROR_DEBUG_BKPT;// нет места в очереди
    #endif
            memset(route_elem, 0, sizeof(Route_elem_t));
        }
    }
    else
    {
        route_elem->index_route_list = route_elem_old->index_route_list;
        MEMCPY((uint8_t*)route_elem_old, (uint8_t*)route_elem, sizeof(Route_elem_t));
        delete_route_elem(route_elem);
    }
}

void F_CLASS delete_route_elem(Route_elem_t *route_elem)
{
    memset(route_elem, 0, sizeof(Route_elem_t));
}

Route_elem_t* F_CLASS create_route_elem(uint16_t dest_sta_addr, uint32_t dest_seq_num,
                                        uint16_t orig_sta_addr, uint32_t orig_seq_num, uint16_t next_hop_addr,
                                        uint32_t rreq_id, uint8_t hop_count, Flags_t flags,
                                        Route_elem_state_t state, uint8_t dest_count, const Unreachable_node_t *unreachable_nodes)
{
    U8_T i_route_elem;
    Route_elem_t *route_elem;

    i_route_elem = alloc_route_elem();

    if (i_route_elem == MAX_SIZE_LIST_ROUTE)
    {
        return NULL;
    }

    route_elem = &serv_msg_route_list[i_route_elem];

    route_elem->dest_sta_addr = dest_sta_addr;
    route_elem->dest_seq_num = dest_seq_num;
    route_elem->orig_sta_addr = orig_sta_addr;
    route_elem->orig_seq_num = orig_seq_num;
    route_elem->rreq_id = rreq_id;
    route_elem->hop_count = hop_count;
    route_elem->next_hop_addr = next_hop_addr;
    route_elem->flags = flags;
    route_elem->state = state;
    route_elem->dest_count = dest_count;
    route_elem->index_route_list = i_route_elem;
    MEMCPY((uint8_t*)route_elem->unreachable_nodes, (uint8_t*)unreachable_nodes, sizeof(Unreachable_node_t)*dest_count);

    return route_elem;
}

Bool_t F_CLASS find_black_list(uint16_t orig_sta_addr, uint32_t id)
{
	U8_T i;

	for (i = 0; i < MAX_SIZE_LIST_ROUTE; i++)
	{
		if ((black_list[i].orig_sta_addr == orig_sta_addr) && (black_list[i].rreq_id == id))
		{
			return TRUE_C;
		}
	}
	return FALSE_C;
}

void F_CLASS add_black_list(uint16_t orig_sta_addr, uint32_t id)
{
    U8_T i;

    for (i = 0; i < MAX_SIZE_LIST_ROUTE; i++)
    {
        if (black_list[i].orig_sta_addr == EMPTY)
        {
            black_list[i].orig_sta_addr = orig_sta_addr;
            black_list[i].rreq_id = id;
            black_list[i].num_frames = TIME_WAIT_RREP_FRAMES;

            return;
        }
    }

#if (DEBUG_ASM_BKPT)
        ERROR_DEBUG_BKPT; // нет места
#endif

}

void F_CLASS remove_black_list(Black_list_elem_t *elem)
{
    memset(elem, EMPTY, sizeof(Black_list_elem_t));
}

void F_CLASS check_remove_black_list()
{
	U8_T i;

	for (i = 0; i < MAX_SIZE_LIST_ROUTE; i++)
	{
		if (black_list[i].orig_sta_addr != EMPTY)// не пустая запись в таблице
		{
			black_list[i].num_frames--;
			if (black_list[i].num_frames == 0)
			{
				remove_black_list(&black_list[i]);
			}
		}
	}
}

Route_row_t* F_CLASS get_route(uint16_t sta_addr)
{
    U8_T i;

    if (sta_addr == EMPTY)
    {
#if (DEBUG_ASM_BKPT)
        ERROR_DEBUG_BKPT; // пустой адрес
#endif
        return NULL;
    }

    for (i = 0; i < MAX_SIZE_TABLE_ROUTE; i++)
    {
        if (routing_table[i].sta_addr == sta_addr)
        {
            return &routing_table[i];
        }
    }
    return NULL;// не нашли
}

Route_row_t* F_CLASS get_active_route(uint16_t sta_addr)
{
    U8_T i;

    if (sta_addr == EMPTY)
    {
#if (DEBUG_ASM_BKPT)
        ERROR_DEBUG_BKPT; // пустой адрес
#endif
        return NULL;
    }

    for (i = 0; i < MAX_SIZE_TABLE_ROUTE; i++)
    {
        if ( (routing_table[i].sta_addr == sta_addr) && (routing_table[i].state == ACTIVE) )
        {
            return &routing_table[i];
        }
    }
    return NULL;// не нашли
}

void F_CLASS add_precursor(Route_row_t *route, uint16_t sta_addr)
{
	U8_T i;

	for (i = 0; i < MAX_RADIO_CONNECTIONS; i++)
	{
		if (route->list_of_precursors[i].sta_addr == sta_addr)
		{
			return;
		}

		if (route->list_of_precursors[i].sta_addr == 0)
		{
			route->list_of_precursors[i].sta_addr = sta_addr;
			return;
		}
	}

	if (i == MAX_RADIO_CONNECTIONS)
	{
#if (DEBUG_ASM_BKPT)
		ERROR_DEBUG_BKPT;
#endif
		return;
	}
}

Bool_t F_CLASS compare_num32(uint32_t num1, uint32_t num2)
{
    if ((int32_t)(num1 - num2) > 0)
    {
        return TRUE_C;
    }
    else
    {
        return FALSE_C;
    };
}

Bool_t F_CLASS compare_num8(uint8_t num1, uint8_t num2)
{
    if ((int8_t)(num1 - num2) > 0)
    {
        return TRUE_C;
    }
    else
    {
        return FALSE_C;
    };
}

Bool_t F_CLASS compare_SNR(uint16_t STA1, uint16_t STA2)
{
    U8_T BI1, BI2;

    BI1 = Find_In_AddressBook(STA1);
    BI2 = Find_In_AddressBook(STA2);

    if ( (BI1 == 0) || (BI2 == 0) )
    {
        return FALSE_C;
    }

    BI1 --;
    BI2 --;

    if (mac.SNR_AS[BI1] > mac.SNR_AS[BI2])
    {
        return TRUE_C;
    }
    else
    {
        return FALSE_C;
    }
}

Net_addr_t* F_CLASS get_as_by_ip_in_net(uint32_t ip_addr)
{
    U8_T i;

    if (ip_addr == EMPTY)
    {
#if (DEBUG_ASM_BKPT)
        ERROR_DEBUG_BKPT; // пустой адрес
#endif
        return NULL;
    }

    for (i = 0; i < MAX_RADIO_CONNECTIONS; i++)
    {
        if (net_addr_table[i].ip_addr == ip_addr)
        {
            return &net_addr_table[i];
        }
    }
    return NULL;
}

Net_addr_t* F_CLASS get_as_by_mac_in_net(eth_addr *mac_addr)
{
    U8_T i;
    eth_addr zero_mac_addr;
    memset(&zero_mac_addr, EMPTY, sizeof(eth_addr));

    if (mac_addr_cmp(mac_addr, &zero_mac_addr) == 0)
    {
#if (DEBUG_ASM_BKPT)
        ERROR_DEBUG_BKPT; // пустой адрес
#endif
        return NULL;
    }

    for (i = 0; i < MAX_RADIO_CONNECTIONS; i++)
    {
        if ( mac_addr_cmp(mac_addr, &net_addr_table[i].mac_addr) == 0 )
        {
            return &net_addr_table[i];
        }
    }
    return NULL;

}

Net_addr_t* F_CLASS get_as_by_addr_in_net(uint16_t sta_addr)
{
    U8_T i;

    if (sta_addr == EMPTY)
    {
#if (DEBUG_ASM_BKPT)
        ERROR_DEBUG_BKPT; // пустой адрес
#endif
        return NULL;
    }

    for (i = 0; i < MAX_RADIO_CONNECTIONS; i++)
    {
        if (net_addr_table[i].sta_addr == sta_addr)
        {
            return &net_addr_table[i];
        }
    }
    return NULL;
}

void F_CLASS set_as_in_net(uint16_t sta_addr, eth_addr *mac_addr, uint32_t ip_addr)
{
    U8_T i;

    //Проверка на mac (не может быть 2 записи с одинаковым mac адресом)
    if (get_as_by_mac_in_net(mac_addr) != NULL)
    {
        return;
    }

    // Пробегаем по таблице в поиске пустой ячейки
    for (i = 0; i < MAX_RADIO_CONNECTIONS; i++)
    {
        if (net_addr_table[i].sta_addr == EMPTY)
        {
            MEMCPY(&(net_addr_table[i].mac_addr.addr[0]), mac_addr->addr, sizeof(eth_addr));
            net_addr_table[i].ip_addr = ip_addr;
            net_addr_table[i].sta_addr = sta_addr;

            // Добавляем собствнный ip
            if (sta_addr == mac.addr)
            {
                mac.ip_addr = ip_addr;
            }

            return;
        }
    }

#if DEBUG_ASM_BKPT
    // Если нет места в таблице
    if (i == MAX_RADIO_CONNECTIONS)
    {
        ERROR_DEBUG_BKPT;
        return;
    }
#endif
}

void F_CLASS try_set_addr_info_in_net(const eth_hdr *eth, uint16_t addr_DATA)
{
    arp_message_t *msg;
    ip_hdr *iphdr;

    // arp
    if (eth->type == PP_HTONS(ETHTYPE_ARP))
    {
        msg = (arp_message_t*) (eth->data);
        set_as_in_net(addr_DATA, (eth_addr*)eth->src.addr, msg->ip_addr_from);

    }
    // ip
    else if (eth->type == PP_HTONS(ETHTYPE_IP))
    {
        iphdr = (ip_hdr *)eth->data;
        set_as_in_net(addr_DATA, (eth_addr*)eth->src.addr, iphdr->src_addr);
    }
}

Bool_t F_CLASS is_own_ip_addr(const eth_hdr *eth)
{
    arp_message_t *msg;
    ip_hdr *iphdr;

    if (mac.ip_addr != 0)
    {
        if (eth->type == PP_HTONS(ETHTYPE_ARP)) // arp
        {
            msg = (arp_message_t*)eth->data;

            if (msg->ip_addr_to == mac.ip_addr)
            {
                return TRUE_C;
            }
        }
        else if (eth->type == PP_HTONS(ETHTYPE_IP)) // ip
        {
            iphdr = (ip_hdr *)eth->data;

            if (iphdr->dest_addr == mac.ip_addr)
            {
                return TRUE_C;
            }
        }
    }

    return FALSE_C;
}

Bool_t F_CLASS is_own_mac_addr(const eth_addr *addr)
{
    Net_addr_t *as_net;
    as_net = get_as_by_addr_in_net(mac.addr);
    if (as_net == NULL)
    {
#if DEBUG_ASM_BKPT
        ERROR_DEBUG_BKPT;// нет записи в таблице о себе
#endif
        return FALSE_C;
    }

    if (memcmp(as_net->mac_addr.addr, addr, ETHARP_HWADDR_LEN) == 0)
    {
        return TRUE_C;
    }
    else
    {
        return FALSE_C;
    }
}

U8_T F_CLASS check_arp_icmp(U8_T pbuf_index)
{
#if (AUTO_REPLY_ARP_ICMP)
    eth_hdr *ethhdr;
    arp_message_t *msg;
    ip_hdr *iphdr;
    icmp_echo_hdr *iecho;
    Net_addr_t *as_net;
    Route_row_t *route;
#endif

    // пытаемся извлечь из пакета mac, ip и сопоставить с собственным адресом
    try_set_addr_info_in_net((eth_hdr*) pbuffs_to_RXTX[pbuf_index].buff, mac.addr);

#if (AUTO_REPLY_ARP_ICMP)

#if defined (__OMNET__)        // OMNET Compiler
    if (!autoreply_arp_icmp)
    {
        return 0;
    }
#endif //defined (__OMNET__)        // OMNET Compiler

    ethhdr = (eth_hdr*) (pbuffs_to_RXTX[pbuf_index].buff);

    if (is_own_mac_addr(&ethhdr->src))
    {
        // arp
        if (ethhdr->type == PP_HTONS(ETHTYPE_ARP))
        {
            msg = (arp_message_t *)ethhdr->data;

            // Отбрасываем Gratuitous arp
            if (msg->ip_addr_to == msg->ip_addr_from)
                return 0;

            // arp req
            if (msg->type == ARP_TYPE_REQUEST)
            {
                as_net = get_as_by_ip_in_net(msg->ip_addr_to);
                if (as_net != NULL)
                {
                	route = get_active_route(as_net->sta_addr);
                    // Автоответы arp только для тех, на кого известен активный маршрут
                    if (route != NULL )
                    {
                        // Создаем arp rep
                        create_arp_autoreply(pbuf_index, as_net);// изменяет msg

                        return 1;
                    }
                }
            }
        }
        // ip
        else if (ethhdr->type == PP_HTONS(ETHTYPE_IP))
        {
            iphdr = (ip_hdr *)ethhdr->data;

            // icmp
            if (iphdr->_proto == IP_PROTO_ICMP)
            {
                // ограничение длины icmp
                if (PP_HTONS(iphdr->_len) <= ICMP_STANDART_TOTAL_LEN)
                {
                    iecho = (icmp_echo_hdr *)iphdr->data;

                    // icmp req
                    if (iecho->type == ICMP_ECHO)
                    {
                        as_net = get_as_by_ip_in_net(iphdr->dest_addr);
                        if (as_net != NULL)
                        {
                            // Автоответы icmp только для тех, чей бикон известен
                            if (Find_In_AddressBook(as_net->sta_addr))
                            {
                                // Создаем icmp rep
                                create_icmp_echo_autoreply(pbuf_index, as_net);// изменяет iphdr

                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    #endif

    return 0;
}

Bool_t F_CLASS hasConnection(uint16_t addr)
{
    U8_T BI;

    // Ищем абонента в адресной таблице
    BI = find_Addr_pos(addr);
    if (BI == 0)
    {
        return FALSE_C; // нет такого абонента (дальний абонент)
    }
    BI--;

    if ( (mac.AddressTable[BI].vsv != VSV_RSV) && (mac.AddressTable[BI].vsv != VSV_PSV) )
    {
        return TRUE_C; //нет связи, это САС для соседа
    }

    return FALSE_C;
}

void F_CLASS create_arp_autoreply(U8_T pbuf_index, const Net_addr_t *as_net_src)
{
    eth_hdr *ethhdr;
    arp_message_t *msg;
    Net_addr_t *as_net_dest;

    pbuffs_to_RXTX[pbuf_index].owner = TO_HIGH_LEVEL;

    ethhdr = (eth_hdr *)(pbuffs_to_RXTX[pbuf_index].buff);

    as_net_dest = get_as_by_addr_in_net(mac.addr);
    if (as_net_dest == NULL)
    {
#if DEBUG_ASM_BKPT
        ERROR_DEBUG_BKPT;
#endif
        return;
    }

    MEMCPY(ethhdr->dest.addr, as_net_dest->mac_addr.addr, ETHARP_HWADDR_LEN);
    MEMCPY(ethhdr->src.addr, as_net_src->mac_addr.addr, ETHARP_HWADDR_LEN);

    msg = (arp_message_t *)(ethhdr->data);
    msg->type = ARP_TYPE_RESPONSE;
    MEMCPY(msg->mac_addr_from, ethhdr->src.addr, ETHARP_HWADDR_LEN);
    msg->ip_addr_from = as_net_src->ip_addr;
    MEMCPY(msg->mac_addr_to, ethhdr->dest.addr, ETHARP_HWADDR_LEN);
    msg->ip_addr_to = as_net_dest->ip_addr;

    if (push_pDataInQueue(pbuf_index, (indexQueue_TypeDef*) MAC_RX_Queue, MAC_RX_QUEUE_SZ) == 0)
    {
#if defined (__OMNET__)        // OMNET Compiler
        Debug_Str_Send(ERROR_String,user_sprintf_int(ERROR_String, "push_pDataInQueue OVF pktlen = %", pbuffs_to_RXTX[pbuf_index].len), EN_DEBUG_BCN_DISTRIBUTION);
#endif //defined (__OMNET__)        // OMNET Compiler

        free_pbuff_to_RXTX (pbuf_index, TO_HIGH_LEVEL);
    }
}

void F_CLASS create_icmp_echo_autoreply(U8_T pbuf_index, const Net_addr_t *as_net_src)
{
    eth_hdr *ethhdr;
    ip_hdr *iphdr;
    icmp_echo_hdr *iecho;
    Net_addr_t *as_net_dest;
    uint32_t checksum = 0;
    uint8_t *pbuf;
    U8_T i;

    pbuffs_to_RXTX[pbuf_index].owner = TO_HIGH_LEVEL;

    pbuf = pbuffs_to_RXTX[pbuf_index].buff;

    ethhdr = (eth_hdr *)(pbuf);

    as_net_dest = get_as_by_addr_in_net(mac.addr);
    if (as_net_dest== NULL)
    {
#if DEBUG_ASM_BKPT
        ERROR_DEBUG_BKPT;
#endif
        return;
    }

    iphdr = (ip_hdr *)(ethhdr->data);
    iphdr->dest_addr = as_net_dest->ip_addr;
    iphdr->src_addr = as_net_src->ip_addr;

    iecho = (icmp_echo_hdr *)(iphdr->data);
    iecho->type = ICMP_ER;      //echo reply
    // adjust the checksum
    iecho->chksum = iecho->chksum + 0x0008;

    iphdr->_ttl = ICMP_TTL;
    iphdr->_chksum = 0;

    pbuf = &pbuf[sizeof(eth_hdr)];
    for (i = 0; i < (sizeof(ip_hdr)); i += 2) //ip_checksum
    {
        checksum += PP_HTONS(*(int16_t*)(&pbuf[i]));
    }
    checksum = ~((checksum & 0xffff) + (checksum >> 16));

    iphdr->_chksum = PP_HTONS(checksum);
    MEMCPY(ethhdr->dest.addr, as_net_dest->mac_addr.addr, ETHARP_HWADDR_LEN);
    MEMCPY(ethhdr->src.addr, as_net_src->mac_addr.addr, ETHARP_HWADDR_LEN);
    ethhdr->type = PP_HTONS(ETHTYPE_IP);

    if (push_pDataInQueue(pbuf_index, (indexQueue_TypeDef*) MAC_RX_Queue, MAC_RX_QUEUE_SZ) == 0)
    {
#if defined (__OMNET__)        // OMNET Compiler
        Debug_Str_Send(ERROR_String,user_sprintf_int(ERROR_String, "push_pDataInQueue OVF pktlen = %", pbuffs_to_RXTX[pbuf_index].len), EN_DEBUG_BCN_DISTRIBUTION);
#endif //defined (__OMNET__)        // OMNET Compiler

        free_pbuff_to_RXTX (pbuf_index, TO_HIGH_LEVEL);
    }
}

void F_CLASS update_or_create_routes_to_neighbors()
{
    int i;
    uint16_t sta_addr;

    for (i = 0; i < MAX_RADIO_CONNECTIONS; i++)
    {
        sta_addr = mac.AddressTable[i].addr;

        if (sta_addr == mac.addr || sta_addr == EMPTY)
        {
            continue;
        }

        update_or_create_previus_route(sta_addr);
    }
}

void F_CLASS update_or_create_previus_route(uint16_t sta_addr)
{
    Route_row_t *route;

    route = get_route(sta_addr);
    if (route == NULL)
    {
        create_route(sta_addr, INVALID, 0, 1, sta_addr, ACTIVE, ACTIVE_ROUTE_TIMEOUT_FRAMES);
    }
    else
    {
        update_route(route, route->seq_num, route->valid_flag, sta_addr, 1, ACTIVE, ACTIVE_ROUTE_TIMEOUT_FRAMES);
    }
}

Bool_t F_CLASS update_or_create_reverse_route(Route_row_t **route_reverse, const Route_row_t *route_previus, const Pkt_RREQ *rreq_pkt)
{
    // двойной указатель потому что create_route() указатель
    Bool_t cond1, cond2, cond3, cond4;

    if (*route_reverse == NULL)
    {
        *route_reverse = create_route(rreq_pkt->orig_sta_addr, VALID, rreq_pkt->orig_seq_num, rreq_pkt->hop_count,
                                        route_previus->sta_addr, ACTIVE, ACTIVE_ROUTE_TIMEOUT_FRAMES);
    }
    else
    {
        // Маршрут обновляется если новый sequence number либо
        // (1) Больше чем destination sequence number в таблице маршрутизации или
        cond1 = compare_num32(rreq_pkt->orig_seq_num, (*route_reverse)->seq_num);

        // (2) при равентве sequence number, hop count + 1 (новый) меньше чем существующий в таблице или
        cond2 = (Bool_t)( ( rreq_pkt->orig_seq_num == (*route_reverse)->seq_num) && (rreq_pkt->hop_count < (*route_reverse)->hop_count) );

        // (3) sequence number неизвестен
        cond3 = (Bool_t)((*route_reverse)->valid_flag == INVALID);

        // (4) При равенстве hop_count, seq_num, сравниваем ОСШ (добавленное)
        cond4 = (Bool_t)( ( rreq_pkt->orig_seq_num == (*route_reverse)->seq_num) && (rreq_pkt->hop_count == (*route_reverse)->hop_count)
                && ( compare_SNR(route_previus->sta_addr, (*route_reverse)->next_hop_addr) ) );

        if (cond1 || cond2 || cond3 || cond4)
        {
            update_route(*route_reverse, rreq_pkt->orig_seq_num, VALID, route_previus->sta_addr, rreq_pkt->hop_count, ACTIVE, ACTIVE_ROUTE_TIMEOUT_FRAMES);

            return TRUE_C;
        }
    }

    return FALSE_C;
}

void F_CLASS update_or_create_forward_route(Route_row_t **route_forward, const Route_row_t *route_previus, const Pkt_RREP *rrep_pkt)
{
    // двойной указатель потому что create_route() указатель
    Bool_t cond1, cond2, cond3, cond4, cond5;

    if (*route_forward == NULL)
    {
        *route_forward = create_route(rrep_pkt->dest_sta_addr, VALID, rrep_pkt->dest_seq_num, rrep_pkt->hop_count,
                                        route_previus->sta_addr, ACTIVE, ACTIVE_ROUTE_TIMEOUT_FRAMES);
    }
    else
    {
        // При сравнении существующая запись обновляется только в следующих случаях:

        // (1) sequence number в таблице маршрутизации INVALID
        cond1 = (Bool_t)((*route_forward)->valid_flag == INVALID);

        // (2) Destination Sequence Number в пакете RREP больше чем в таблице маршрутизации
        cond2 = compare_num32(rrep_pkt->dest_seq_num, (*route_forward)->seq_num);

        // (3) sequence numbers равны, но маршрут не активен
        cond3 = (Bool_t)( (rrep_pkt->dest_seq_num == (*route_forward)->seq_num)
        				&& ( ((*route_forward)->state == INACTIVE) || ((*route_forward)->state == WAIT_RREP) )
						 );

        // (4) sequence numbers равны, но hop_count в пакете RREP меньше чем в таблице
        cond4 = (Bool_t)( ( rrep_pkt->dest_seq_num == (*route_forward)->seq_num) && (rrep_pkt->hop_count < (*route_forward)->hop_count) );

        // (v) При равенстве hop_count, seq_num, сравниваем ОСШ (добавленное)
        cond5 = (Bool_t)( ( rrep_pkt->dest_seq_num == (*route_forward)->seq_num) && (rrep_pkt->hop_count == (*route_forward)->hop_count)
                            && ( compare_SNR(route_previus->sta_addr, (*route_forward)->next_hop_addr) ) );

        if (cond1 || cond2 || cond3 || cond4 || cond5)
        {
            update_route(*route_forward, rrep_pkt->dest_seq_num, VALID, route_previus->sta_addr, rrep_pkt->hop_count, ACTIVE, ACTIVE_ROUTE_TIMEOUT_FRAMES);
        }
    }
}

void F_CLASS Proc_PKT_RREQ(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr)
{
	Pkt_RREQ *rreq_pkt;
    Route_row_t *route_previus, *route_reverse, *route_forward;
    //route_reverse - route to orig addr; //route_forward - route to dest addr; route_previus может быть = route_forward

    Bool_t found;
    Bool_t route_reverse_updated = FALSE_C;
	Route_elem_t *route_elem;
	Flags_t flags;

	if (mac.state != CONNECTED)
	{
		return;
	}

	memset(&flags, 0, sizeof(Flags_t));

	rreq_pkt = (Pkt_RREQ*)type_Hdr;

    // Добавляем пакет RREQ в black_list, чтобы отклонять ретранслируемые пакеты
    // но так как у соседей есть порядок передачи, то первый пришедший пакет RREQ
    // не значит оптимальный путь поэтому обрабатываем каждый RREQ, который обновляет обратный маршрут
    found = find_black_list(rreq_pkt->orig_sta_addr, rreq_pkt->rreq_id);
    if (!found)
    {
    	add_black_list(rreq_pkt->orig_sta_addr, rreq_pkt->rreq_id);// Буферизируем RREQ
    }

    // Свои запросы REQ отбрасываем
    if (rreq_pkt->orig_sta_addr == mac.addr)
    {
    	return;
    }

    // Обновляем или создаем обратный маршрут
    rreq_pkt->hop_count++;
    route_previus = get_route(PKTdata->SrcAddr);
    if (route_previus == NULL)
    {
        return;
    }
    route_reverse = get_route(rreq_pkt->orig_sta_addr);
    route_reverse_updated = update_or_create_reverse_route(&route_reverse, route_previus, rreq_pkt);

    // Узел генерирует пакет RREP если
    // (1) он является destination node
    // (2) он имеет активный маршрут до destination node и в таблице маршрутизации destination sequence number
    // является VALID и больше или равен destination sequence number в RREQ (comparison sing signed 32-bit arithmetic)
    // и флаг 'D' не установлен

	// Так как у соседей есть порядок передачи, то первый REQ не обязательно оптимальный.
	// Поэтому отвечаем на этот REQ или ретранслируем этот REQ, если маршрут оптимальней (обновился выше)
    if ( !found || route_reverse_updated )
    {
        route_forward = get_route(rreq_pkt->dest_sta_addr);

        // check (i) destination node
        if (rreq_pkt->dest_sta_addr == mac.addr)
        {
            // create and send RREP
            if ( compare_num32(rreq_pkt->dest_seq_num, mac.seq_num) )
                mac.seq_num = rreq_pkt->dest_seq_num;

            route_elem = create_route_elem(mac.addr, mac.seq_num,
                                            route_reverse->sta_addr, NOT_USED, route_reverse->next_hop_addr,
                                            NOT_USED, 0, flags,
                                            SEND_RREP, NOT_USED, NOT_USED);
            send_route_elem(route_elem);
        }
        // check (ii) an intermediate node
        else if (
				    (route_forward != NULL)
				    && (route_forward->state == ACTIVE)
				    && ( (compare_num32(route_forward->seq_num, rreq_pkt->dest_seq_num) )
				            || (route_forward->seq_num == rreq_pkt->dest_seq_num) )
                    && (rreq_pkt->dest_only_flag == 0)
                    && (route_forward->valid_flag == VALID)
		)
		{
            // create and send RREP
            if (
                    (route_reverse->sta_addr != route_forward->next_hop_addr) &&
                    // Для уменьшения трафика не отвечаем на REQ, если маршрут до dest который известен этой АС, лежит через источник REQ
                    (route_forward->sta_addr != route_reverse->next_hop_addr)
                    // Маршрут RREP не должен проходить через адрес назначения RREQ (создается маршрут на себя)
            )
            {
                route_elem = create_route_elem(route_forward->sta_addr, route_forward->seq_num,
                                                route_reverse->sta_addr, NOT_USED, route_reverse->next_hop_addr,
                                                NOT_USED, route_forward->hop_count, flags,
                                                SEND_RREP, NOT_USED, NOT_USED);
                send_route_elem(route_elem);

                // Если RREQ имеет установленный флаг 'G', а промежуточный узел отправляет RREP originating node,
                // он также ДОЛЖЕН отправить GREP destination node как если бы destination node уже выдал RREQ для originating node
                // При GRREP destAddr меняется на originatorAddr и наоборот
                if (rreq_pkt->gratuitous_RREP_flag == 1)
                {
					// Маршрут GRREP не должен проходить через адрес отправителя RREQ (создается маршрут на себя)
                    route_elem = create_route_elem(route_reverse->sta_addr, route_reverse->seq_num,
                                                    route_forward->sta_addr, NOT_USED, route_forward->next_hop_addr,
                                                    NOT_USED, route_reverse->hop_count, flags,
                                                    SEND_GRREP, NOT_USED, NOT_USED);
                    send_route_elem(route_elem);
                }
            }
        }
        else // rebroadcast RREQ
        {
            // Если узел не генерирует RREP, то он ретранслирует RREQ широковещательно
            flags.join = rreq_pkt->join_flag;
            flags.repair = rreq_pkt->repair_flag;
            flags.gratuitous_RREP = rreq_pkt->gratuitous_RREP_flag;
            flags.dest_only = rreq_pkt->dest_only_flag;
            flags.unknown_seq_num = rreq_pkt->unknown_seq_num_flag;

            route_elem = create_route_elem(rreq_pkt->dest_sta_addr, rreq_pkt->dest_seq_num,
                                            rreq_pkt->orig_sta_addr, rreq_pkt->orig_seq_num, SOFT_ADDR_BRDCST,
                                            rreq_pkt->rreq_id, rreq_pkt->hop_count, flags,
                                            SEND_RREQ, NOT_USED, NOT_USED);
            send_route_elem(route_elem);
        }
    }
}

void F_CLASS Proc_PKT_RREP(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr)
{
	Pkt_RREP *rrep_pkt;
	Route_elem_t *route_elem;
	Route_row_t *route_previus, *route_reverse, *route_forward;
	//route_reverse - route to orig addr; //route_forward - route to dest addr; route_previus может быть = route_forward

	Flags_t flags;

    if (mac.state != CONNECTED)
    {
        return;
    }

	memset(&flags, 0, sizeof(Flags_t));

	rrep_pkt = (Pkt_RREP*)type_Hdr;

    // Обновляем или создаем прямой маршрут
    rrep_pkt->hop_count++;
    route_previus = get_route(PKTdata->SrcAddr);
    if (route_previus == NULL)
    {
        return;
    }
    route_forward = get_route(rrep_pkt->dest_sta_addr);

    update_or_create_forward_route(&route_forward, route_previus, rrep_pkt);

    // RREP адресован не к нам
	if (rrep_pkt->orig_sta_addr != mac.addr)
	{
	    route_reverse = get_route(rrep_pkt->orig_sta_addr);

        if (route_reverse == NULL)
        {
#if (DEBUG_ASM_BKPT)
            ERROR_DEBUG_BKPT; // нет маршрута на абонента
#endif
        	return;
        }

        if (route_forward->sta_addr != route_reverse->next_hop_addr) // мы не пересылаем RREP абоненту dest addr, который сам генерирует такие ответы (ситуация когда кто то отправил GRREP)
        {
            // Пересылаем RREP дальше по цепочке

            // Если узел пересылает RREP по каналу, который, вероятно, содержит ошибки или является однонаправленным,
            // узлу СЛЕДУЕТ установить флаг «A», чтобы требовать, чтобы получатель RREP подтверждал получение RREP, отправляя обратно сообщение RREP-ACK
            if (0)//todo доделать если надо
            {
                rrep_pkt->ack_required_flag = 1;
            }

            // Когда какой-либо узел передает RREP, список предшественников для соответствующего узла назначения обновляется
            // путем добавления к нему узла следующего перехода, на который пересылается RREP.
            add_precursor(route_forward, route_reverse->next_hop_addr);

            // Наконец, список предшественников для следующего перехода к месту назначения обновляется,
            // чтобы содержать следующий переход к источнику (инициатору).
            add_precursor(route_reverse, route_forward->next_hop_addr);

            // Rebroadcast RREP
            flags.repair = rrep_pkt->repair_flag;
            flags.ack_required = rrep_pkt->ack_required_flag;

            route_elem = create_route_elem(rrep_pkt->dest_sta_addr, rrep_pkt->dest_seq_num,
                                            rrep_pkt->orig_sta_addr, NOT_USED, route_reverse->next_hop_addr,
                                            NOT_USED, rrep_pkt->hop_count, flags,
                                            SEND_RREP, NOT_USED, NOT_USED);
            send_route_elem(route_elem);
        }
	}
}

void F_CLASS Proc_PKT_RERR(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr)
{
    Pkt_RERR *rerr_pkt;
    U8_T i_route, i, dest_count = 0;
    Route_elem_t *route_elem;
    Route_row_t *route;
    Flags_t flags;
    Unreachable_node_t unreachable_nodes[MAX_RADIO_CONNECTIONS];
    Unreachable_node_t recieved_node;

    if (mac.state != CONNECTED)
    {
        return;
    }

    memset(&flags, 0, sizeof(Flags_t));

    rerr_pkt = (Pkt_RERR*)type_Hdr;

    // Узел инициирует обработку сообщения RERR в трех случаях (по протоколу):
    // У нас только 3ий пункт
    // (3) Если узел получает RERR от соседа для одного или нескольких активных маршрутов

    for (i_route = 0; i_route < MAX_SIZE_TABLE_ROUTE; i_route++)
    {
        route = &routing_table[i_route];

        if (route->sta_addr == EMPTY)
        {
            continue;
        }
        // Дополняем список недоступных узлов, если источник RERR указан в качестве следующего перехода в таблице маршрутизации
        if ( (route->state == ACTIVE) && (route->next_hop_addr == PKTdata->SrcAddr) )
        {
            for (i = 0; i < rerr_pkt->dest_count; ++i)
            {
                recieved_node = rerr_pkt->unreachable_nodes[i];

                if (recieved_node.sta_addr == route->sta_addr)
                {
                    unreachable_nodes[dest_count].sta_addr = recieved_node.sta_addr;

                    if (recieved_node.valid_flag == VALID)
                    {
                        route->seq_num = recieved_node.seq_num;
                        route->valid_flag = VALID;
                        unreachable_nodes[dest_count].seq_num = recieved_node.seq_num;
                        unreachable_nodes[dest_count].valid_flag = VALID;
                    }
                    else if (route->valid_flag == VALID)
                    {
                        unreachable_nodes[dest_count].seq_num = ++routing_table[i].seq_num;
                        unreachable_nodes[dest_count].valid_flag = VALID;
                    }
                    else
                    {
                       unreachable_nodes[dest_count].valid_flag = INVALID;
                    }

                    set_route_state(route, INACTIVE);
                    dest_count++;
                }
            }
        }
    }

    if (dest_count != 0)
    {

        route_elem = create_route_elem(NOT_USED, NOT_USED,
                                        mac.addr, NOT_USED, SOFT_ADDR_BRDCST,
                                        NOT_USED, NOT_USED, flags,
                                        SEND_RERR, dest_count, unreachable_nodes);
        send_route_elem(route_elem);
    }

}

void F_CLASS Proc_PKT_mac_ip_req(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr)
{
	Pkt_mac_ip_req *pkt_req;
	Route_elem_t *route_elem;
	Route_row_t *route;
	Flags_t flags;
	Route_elem_state_t state;

	if (mac.ip_addr != 0)
	{
		memset(&flags, 0, sizeof(Flags_t));

		pkt_req = (Pkt_mac_ip_req*)type_Hdr;

		set_as_in_net(pkt_req->src_sta_addr, &pkt_req->src_mac_addr, pkt_req->src_ip_addr);

		if (mac.addr == pkt_req->dest_sta_addr)
		{
			state = SEND_MAC_IP_REP;
			route = get_active_route(pkt_req->src_sta_addr);
		}
		else // rebroadcast
		{
			state = SEND_MAC_IP_REQ;
			route = get_active_route(pkt_req->dest_sta_addr);
		}

		if (route == NULL)
		{
			return;
		}

		route_elem = create_route_elem(pkt_req->dest_sta_addr, NOT_USED,
										pkt_req->src_sta_addr, NOT_USED, route->next_hop_addr,
										NOT_USED, NOT_USED, flags,
										state, NOT_USED, NOT_USED);
		send_route_elem(route_elem);
	}
}

void F_CLASS Proc_PKT_mac_ip_rep(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr)
{
    Pkt_mac_ip_rep *pkt_rep;
    Route_elem_t *route_elem;
    Route_row_t *route;
    Flags_t flags;

    memset(&flags, 0, sizeof(Flags_t));

    pkt_rep = (Pkt_mac_ip_rep*)type_Hdr;

    set_as_in_net(pkt_rep->dest_sta_addr, &pkt_rep->dest_mac_addr, pkt_rep->dest_ip_addr);

    if (mac.addr != pkt_rep->src_sta_addr) // rebroadcast
    {
        route = get_active_route(pkt_rep->src_sta_addr);

        if (route == NULL)
        {
#if (DEBUG_ASM_BKPT)
            ERROR_DEBUG_BKPT; // нет маршрута на абонента
#endif
            return;
        }

        route_elem = create_route_elem(pkt_rep->dest_sta_addr, NOT_USED,
                                        pkt_rep->src_sta_addr, NOT_USED, route->next_hop_addr,
                                        NOT_USED, NOT_USED, flags,
                                        SEND_MAC_IP_REP, NOT_USED, NOT_USED);
        send_route_elem(route_elem);
    }
}

void F_CLASS Proc_PKT_Request_need_net_map(const Pkt_Hdr *PKTdata)
{
    Route_elem_t *route_elem;
    Flags_t flags;

    memset(&flags, 0, sizeof(Flags_t));

    route_elem = create_route_elem(NOT_USED, NOT_USED,
                                    PKTdata->SrcAddr, NOT_USED, PKTdata->SrcAddr,
                                    NOT_USED, NOT_USED, flags,
                                    SEND_NET_MAP, NOT_USED, NOT_USED);
    send_route_elem(route_elem);
}

void F_CLASS Proc_PKT_net_map(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr)
{
    U8_T i;
    Pkt_net_map *net_map;
    Route_row_t *route;

    net_map = (Pkt_net_map*)type_Hdr;

    for (i = 0; i < net_map->size; i++)
    {
        route = get_route(net_map->sta_addr[i]);

        if (route == NULL)
        {
            create_route(net_map->sta_addr[i], INVALID, 0, 0, 0, INACTIVE, 0);
        }
    }
}

#if defined (__OMNET__)        // OMNET Compiler
} // namespace inet
#endif


