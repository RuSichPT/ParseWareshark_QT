/*
 * Rout_control_pkt.h
 *
 *  Created on: 18 дек. 2020 г.
 *      Author: Токарев Павел
 */

#ifndef ROUT_CONTROL_PKT_H_
#define ROUT_CONTROL_PKT_H_

#include "MAC/Inc/MAC_pkts_format.h"// Pkt_Hdr, Pkt_type_Hdr, Unreachable_node_t,Valid_flag_t
#include "MAC/Inc/MAC_Timeline.h" // MAX_RADIO_CONNECTIONS, TIME_WORK_PERIOD
#include "pbuffs.h" //QUEUE_TRUE_SZ
#include "eth.h"

#define START_STA_ADDR						(100)
#define MAX_SIZE_TABLE_ROUTE				(MAX_RADIO_CONNECTIONS)//максимальный размер таблицы маршрузиции
#define MAX_SIZE_LIST_ROUTE					(MAX_RADIO_CONNECTIONS*4)// максимальный размер списка временных маршрутов
#define SERV_MSG_ROUT_QUEUE_SZ				(QUEUE_TRUE_SZ(3*MAX_SIZE_TABLE_ROUTE)) // Queue for TX service rout msg

#define NET_DIAMETER						(35) // узлов,
#define NODE_TRAVERSAL_TIME					(40) // мс, отведенное время распространения до узла
#define NET_TRAVERSAL_TIME					(2 * NODE_TRAVERSAL_TIME * NET_DIAMETER)// 2800 мс, отведенное время распространения в сети
#define PATH_DISCOVERY_TIME					( (2 * NET_TRAVERSAL_TIME ) / ( TIME_WORK_PERIOD/10000 ) )// во фреймах 56
											// TIME_WORK_PERIOD =  1000000 сотни нс(10^-8) = 100 мс// время в течение которого отклоняются запросы REQ
#define TIME_WAIT_RREP_FRAMES				PATH_DISCOVERY_TIME
#define ACTIVE_ROUTE_TIMEOUT_FRAMES			(100) // во фреймах
#define EMPTY								(0)
#define NOT_USED							(0)
#define TIME_WAIT_MAC_IP_REP_FRAMES			(5)

typedef enum
{
	FALSE_C		= 0,
	TRUE_C		= 1,
} Bool_t;

typedef enum
{
	NOT_STATE				,
	SEND_RREQ				,
	SEND_RREP				,
	SEND_GRREP				,
	SEND_RERR				,
	SEND_MAC_IP_REQ			,
	SEND_MAC_IP_REP			,
	SEND_NET_MAP			,
} Route_elem_state_t;

typedef enum
{
	INACTIVE				,
	ACTIVE					,
	WAIT_RREP				,
	WAIT_DELETE				,
	WAIT_RREPACK			// not used
} Route_state_t;

typedef struct {
	uint16_t				sta_addr;
} Precursors_t;

typedef struct {
	uint16_t				sta_addr;
	Route_state_t			state;
	Valid_flag_t			valid_flag;
	uint32_t				seq_num;
	uint32_t				lifetime;
	Precursors_t			list_of_precursors[MAX_RADIO_CONNECTIONS];
	uint16_t				next_hop_addr;
	uint8_t					hop_count;
} Route_row_t;

typedef struct
{
	uint8_t 				join				:1;  // RREQ
	uint8_t					repair				:1;  // RREP  RREQ
	uint8_t					gratuitous_RREP		:1;  // RREQ
	uint8_t					dest_only			:1;  // RREQ
	uint8_t					unknown_seq_num		:1;  // RREQ
	uint8_t					ack_required		:1;  // RREP
	uint8_t					no_delete			:1;  // RERR
} Flags_t;

typedef struct
{
	uint16_t				dest_sta_addr;
	uint16_t				orig_sta_addr;
	uint32_t				dest_seq_num;
	uint32_t				orig_seq_num;
	uint32_t				rreq_id;
	uint16_t				next_hop_addr;
	uint8_t					hop_count;
	Flags_t					flags;
	Route_elem_state_t		state;
	uint8_t					dest_count;			// счетчик unreachable_nodes
	uint8_t					index_route_list;
	Unreachable_node_t		unreachable_nodes[MAX_RADIO_CONNECTIONS];
} Route_elem_t;

typedef struct
{
	uint32_t 				rreq_id;
	uint32_t 				orig_sta_addr;
	uint32_t				num_frames;
} Black_list_elem_t;

typedef struct
{
	uint32_t				ip_addr;
	uint16_t				sta_addr;
	eth_addr				mac_addr;
} Net_addr_t;

extern Route_row_t routing_table[MAX_SIZE_TABLE_ROUTE];

#if !defined (__OMNET__)        // OMNET Compiler
//!!!CONVERTER_START_FIELD_->

void init_routing();

void handle_routing();

void tick_route_lifetime();

void check_deletion_routes();

void search_routes();

void search_mac_ip();

uint8_t get_unreachable_nodes(Unreachable_node_t *unreachable_nodes, const Route_row_t *unreachable_route);

Route_row_t* create_route(uint16_t addr, Valid_flag_t flag, uint32_t seq_num, uint8_t hop_count,
                            uint16_t next_hop_addr, Route_state_t state, uint32_t lifetime);

void update_route(Route_row_t *route, uint32_t seq_num, Valid_flag_t valid_state, uint16_t next_hop_addr,
                    uint8_t hop_count, Route_state_t state, uint32_t lifetime);

void set_route_state(Route_row_t *route, Route_state_t state);

U8_T alloc_route_elem();

Route_elem_t* find_route_elem(const Route_elem_t *route_elem);

void send_route_elem(Route_elem_t *route_elem);

void delete_route_elem(Route_elem_t *route_elem);

Route_elem_t* create_route_elem(uint16_t dest_sta_addr, uint32_t dest_seq_num,
                                uint16_t orig_sta_addr, uint32_t orig_seq_num, uint16_t next_hop_addr,
                                uint32_t rreq_id, uint8_t hop_count, Flags_t flags,
                                Route_elem_state_t state, uint8_t dest_count, const Unreachable_node_t *unreachable_nodes);

Bool_t find_black_list(uint16_t orig_sta_addr, uint32_t id);

void add_black_list(uint16_t orig_sta_addr, uint32_t id);

void remove_black_list(Black_list_elem_t *elem);

void check_remove_black_list();

Route_row_t* get_route(uint16_t sta_addr);

Route_row_t* get_active_route(uint16_t sta_addr);

void add_precursor(Route_row_t *route, uint16_t sta_addr);

Bool_t compare_num32(uint32_t num1, uint32_t num2);

Bool_t compare_num8(uint8_t num1, uint8_t num2);

Bool_t compare_SNR(uint16_t STA1, uint16_t STA2);

Net_addr_t* get_as_by_ip_in_net(uint32_t ip_addr);

Net_addr_t* get_as_by_mac_in_net(eth_addr *mac_addr);

Net_addr_t* get_as_by_addr_in_net(uint16_t sta_addr);

void set_as_in_net(uint16_t sta_addr, eth_addr *mac_addr, uint32_t ip_addr);

void try_set_addr_info_in_net(const eth_hdr *eth, uint16_t addr_DATA);

Bool_t is_own_ip_addr(const eth_hdr *eth);

Bool_t is_own_mac_addr(const eth_addr *addr);

U8_T check_arp_icmp(U8_T pbuf_index);

Bool_t hasConnection(uint16_t addr);

void send_mac_ip_req();

void create_arp_autoreply(U8_T pbuf_index,const Net_addr_t *as_net_src);

void create_icmp_echo_autoreply(U8_T pbuf_index, const Net_addr_t *as_net_src);

void update_or_create_routes_to_neighbors();

void update_or_create_previus_route(uint16_t sta_addr);

Bool_t update_or_create_reverse_route(Route_row_t **route_reverse, const Route_row_t *route_previus, const Pkt_RREQ *rreq_pkt);

void update_or_create_forward_route(Route_row_t **route_forward, const Route_row_t *route_previus, const Pkt_RREP *rrep_pkt);

void Proc_PKT_RREQ(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr);

void Proc_PKT_RREP(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr);

void Proc_PKT_RERR(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr);

void Proc_PKT_mac_ip_req(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr);

void Proc_PKT_mac_ip_rep(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr);

void Proc_PKT_Request_need_net_map(const Pkt_Hdr *PKTdata);

void Proc_PKT_net_map(const Pkt_Hdr *PKTdata, const Pkt_type_Hdr *type_Hdr);

//!!!CONVERTER_END_FIELD_<-
#endif

#endif /* ROUT_CONTROL_PKT_H_ */
