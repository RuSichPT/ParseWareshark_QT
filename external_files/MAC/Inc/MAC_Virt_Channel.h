
//MAC_Virt_Channel.h//

//Define to prevent recursive inclusion
#ifndef __MAC_VIRT_CHANNEL_H
#define __MAC_VIRT_CHANNEL_H

#include "external_files/MAC/Inc/MAC_debug.h"
#include "external_files/MAC/Inc/MAC_Timeline.h" // SLOTS_PER_ALL_COMMON_TX_INTERVAL
#include "external_files/MAC/Inc/ModemProtocol.h" // ModemRate_t
#include "external_files/pbuffs.h"

#define MAX_VIRT_CH			  				(40)
#define MAX_VIRT_CH_NUM						(255)
#define MAX_VIRT_CH_BRIDGE					(2 * MAX_VIRT_CH) // todo определиться с макс. кол-вом записей
#define MAX_RESERVATION_ENTRY   			(100) // SLOTS_PER_ALL_COMMON_TX_INTERVAL - warning: variably modified 'reserv_elem_list' at file scope
#define MAX_NUM_LOCK_ENTRIES			  	(20)

#define DATA_MAP_QUEUE_SZ					(QUEUE_TRUE_SZ(MAX_VIRT_CH_BRIDGE)) // Queue for TX DATA_MAP
#define DATA_MAP_REQ_QUEUE_SZ				(QUEUE_TRUE_SZ(MAX_VIRT_CH)) 		// Queue for TX DATA_MAP_REQ
#define SERV_MSG_QUEUE_SZ					(QUEUE_TRUE_SZ(MAX_VIRT_CH_BRIDGE)) // Queue for TX service msg

#define MAX_NUM_CHAIN_BUFFERS_PER_VC		(2)
#define MAX_NUM_CHAIN_BUFFERS				(MAX_VIRT_CH * MAX_NUM_CHAIN_BUFFERS_PER_VC)
#define MAX_BUFFERS_IN_CHAIN				(3) //сколько максимум может содержаться буфферов цепочном буфере delay_buffer

#define BYTES_THRESHOLDS					(6)
#define THR_BYTES_1							(64)
#define THR_BYTES_2							(128)
#define THR_BYTES_3							(256)
#define THR_BYTES_4							(512)
#define THR_BYTES_5							(1024)
#define THR_BYTES_6							(ETH_FRAME_SIZE)
#define CYCLES_TO_CALC_VC_BW				(5)//по какому количеству циклов считается средняя пропускная способность виртуального канала
#define BLOCKS_THRESHOLD					(5)//порог по блокам, который нужно пересечь трафику, чтобы вызвать запрос на расширение/сужение канала

#define NUM_CYCLES_FOR_WAIT_VCH_READY	    (2)	// ! *(размер цикла), sizeof(COUNTER) = 1B ! по истечению этого времени повторяем VCH_READY

enum
{
	TX_READINESS	= 0,
	RX_READINESS	= 1,
	READINESS_SIZE	= 2
};

#pragma pack(push, 1)

typedef enum SIZE_OF_ENUM_UINT8
{
	VCH_FUNC_NOTHING					,
	VCH_FUNC_SEND_RTS 					,  // нужно отправить RTS
	VCH_FUNC_SEND_CTS 					,  // нужно отправить CTS
	VCH_FUNC_SEND_CTS_ALT 				,  // нужно отправить CTS со своим вариантом резервирования
	VCH_FUNC_SEND_ACK 					,  // нужно отправить ACK на широковещательный CTS
	VCH_FUNC_SEND_NACK_CLOSING 			,  // нужно отправить NACK для закрытия вирт. канала
	VCH_FUNC_SEND_NACK_CLOSING_RESP		,  // нужно отправить ответный NACK для закрытия вирт. канала
	VCH_FUNC_SEND_NACK_CLOSING_REPEAT	,  // нужно повторить NACK для закрытия вирт. канала, если инф-ции о ВК уже нет
	VCH_FUNC_SEND_NACK_REDUCING 		,  // нужно отправить NACK для уменьшения полосы вирт. канала
	VCH_FUNC_SEND_NACK_ERROR 			,  // нужно отправить NACK в случае ошибочного запроса на резервирование
	VCH_FUNC_SEND_NACK_NOT_EXISTS 		,  // нужно отправить NACK в случае отсутствии вирт. канала
	VCH_FUNC_WAIT_CTS 					,  // ждем CTS
	VCH_FUNC_WAIT_ACK					,  // ждем ACK после отправки CTS
	VCH_FUNC_WAIT_NACK					,  // ждем ответный NACK, если он не был принят, повторяем NACK
	VCH_FUNC_DELETE						,  // помечаем канал, чтобы удалить его в начале следующего фрейма
	VCH_FUNC_WAIT_MERGE_DISTR			,  // ждем конца фрейма, чтобы применить планируемые ЗоР

} PACKED Virt_ch_func_t;

typedef union
{
	uint8_t U;
	struct
	{
		uint8_t channel_participant	  			: 1; // признак того, что АС является участником вирт. канала (слышала RTS в свой адрес)
		uint8_t need_to_send_req_DATA_MAP		: 1; // нужно послать пакет DATA_MAP_REQUEST
		uint8_t reserv 							: 6;
	};
} Vch_Add_States_t;

typedef enum SIZE_OF_ENUM_UINT8
{
	VCH_ST_NOT_READY		= 0,
	VCH_ST_READY			= 1,	// по ВК можно передавать/принимать данные
} PACKED Vch_Ready_State_t;

typedef enum SIZE_OF_ENUM_UINT8
{
	VCH_R_NOTHING,

	VCH_R_SEND_NOT_READY_MSG	,	// уведомляем участников ВК о его неготовности
	VCH_R_WAIT_NON_READY_RESP	,	// ожидаем уведомления на NOT_READY

	VCH_R_NEED_SEND_READY_MSG	,	// нужно уведомить участников ВК о его готовности
	VCH_R_SEND_READY_MSG		,	// уведомляем участников ВК о его готовности
	VCH_R_WAIT_READY_RESP		,	// ожидаем уведомления на READY
	VCH_R_WAIT_READINESS		,	// ожидаем согласование участка ВК
} PACKED Vch_Ready_Func_t;

typedef struct
{
	Vch_Ready_Func_t	state		: 6;	// состояние с-мы готовности
	Vch_Ready_State_t	ready		: 1;	// 1 - готовы посылать/принимать
	uint8_t				response	: 1;	// 1 - формируем ответное сообщ-е
	uint8_t				repeat_cntr;		// по истечению данного числа фреймов повторяем VCH_READY/VCH_NOT_READY
} PACKED Vch_Readiness_t;

#pragma pack(pop)

typedef struct Chain_buffer_elem_t
{
	struct Chain_buffer_elem_t		*next; 		  // указатель на след. элемент со слот-буфером
	uint8_t						  	is_allocated; // признак того что буфер занят
	uint8_t							pbuf_index;
	uint16_t						start_len;
	uint16_t						end_len;
} Chain_buffer_elem_t;

typedef struct Virt_channel_t Virt_ch_t;

typedef struct Reserv_element_t
{
	Reservation_entry_t			RE;				// reserv_entry параметры ЗоР
	struct Reserv_element_t 	*next;			// указатель на следующую ЗоР
	struct Virt_ch_Bridge_t 	*to_bridge;		// указатель на участок ВК, которому принадлежат ЗоР
	Chain_buffer_elem_t 		*slot_buffer;	// указатель на слот-буфер
	struct Reserv_element_t		*to_blocked;	// указатель на заблокированный участок / указатель на следующий на заблокированный участок в blocked_head_elem
	uint16_t					data_len;		// длина данных, размещенных в слот-буферах под данную ЗоР

	uint8_t 					task_idx;		// индекс задачи на передачу/прием +1!

	uint8_t 					blocked		: 1;	// ЗоР используется для хранения заблокированного участка
	uint8_t 					involved	: 1;	// в помеченных ТЗоР ведется передача данных и там невозможен прием служ. сообщ.
	uint8_t 					released	: 1;	// указывает на то, что ЗоР не связаны с ВД, и там можно резервировать или отправлять служебку(если там не запрещено через "blocked_head_elem")
													// (при закрытии канала/ошибочном резервировании/уменьшении ширины вирт. канала)
	uint8_t 					reserved	: 5;
} Reserv_elem_t;

typedef struct Virt_ch_Bridge_t
{
	struct Virt_ch_Bridge_t	*next;
	Virt_ch_t 				*to_virt_ch;

	uint16_t 				dest_addr;				// адрес конечного абонента / промежуточной АС (для ретрансляции)
	uint16_t 				src_addr;				// адрес инициатора обмена / промежуточной АС (для ретрансляции)

	uint8_t					is_allocated; 			// признак того что мост выделен под ВК
	uint8_t					da_infosrc			:4; // информация об адресе получателя была получена лично
	uint8_t					sa_infosrc			:4; // информация об адресе источника была получена лично
	Virt_ch_func_t 			function;				// запланированное действие (TX/RX: RTS/CTS/ACK/NACK и т.п.)
	uint8_t 				wait_time;				// время ожидания какого-либо события (в фреймах), по истечению которого нужно предпринимать меры в зав-ти от текущей ф-ции

	Reserv_elem_t 			*curr_reserv_elem;		// указатель на записи о резервировании
	Reserv_elem_t 			*planned_reserv_elem;	// указатель на планируемые записи о резервировании
	Reserv_elem_t 			*secondary_reserv_elem;	// указатель на записи, которые нужно отправить в NACK, чтобы уменьшить размер вирт. канала

	uint8_t					session_id;				// номер сессии организации или изменения участка ВК
	uint8_t					locking; 				// участок блокирует резервирование другого участка
	uint8_t					locked;
} Vch_Bridge_t;

struct Virt_channel_t
{
	uint16_t 				dest_addr;				// адрес конечного абонента
	uint16_t 				src_addr;				// адрес инициатора обмена (если адрес пустой, то запись ВК пустая)
	uint8_t 				virt_ch_num;			// номер канала
	Traff_Priority_t 		priority		:2;		// приоритет
	TraffDirectionType		direction		:2;		// направление (00-Резерв, 01-Прм, 02-Прд, 03-дуплекс )
	Traff_Type_t 			traffic_type	:4;		// тип трафика

	uint16_t 				gateway;				// адрес промежуточной АС в сторону конечного абонента
	Vch_Add_States_t		flags;
	uint8_t 				time_before_closing;	// по истечению данного числа фреймов удаляем вирт. канал [0 - зарезервирован, удаление при 1]

	union
	{
		struct
		{
			Vch_Readiness_t tx_readiness; 			// состояние информирования о готовности вирт. канала (отправка доп. сообщ. при ретрансляции)
			Vch_Readiness_t rx_readiness;
		};
		Vch_Readiness_t 	readiness[READINESS_SIZE];
	};

	FragmentType			fragmentation;
	uint8_t 				_reserved;

	Vch_Bridge_t			*bridge;				// участки вирт. канала

	Chain_buffer_elem_t 	*delay_buffer;			// цепочка буферов с данными, которые нельзя отбрасывать

	uint16_t				data_per_cycle[CYCLES_TO_CALC_VC_BW];	// объем данных за цикл, полученных с верхнего уровня
	uint8_t					max_cycle_num_reached;//в cycle_num достигнуто значение CYCLES_TO_CALC_VC_BW
	uint8_t					cycle_num;
	uint16_t				need_bytes;
	//инициализирующее зануление происходит перед созданием канала через memset((uint8_t *)&buf_virt_ch, 0, sizeof(Virt_ch_t));
};

typedef struct
{
	uint8_t broadcast; // request for all stations
	uint8_t counter[MAX_RADIO_CONNECTIONS]; // 0 - disabled, 1 - need to send Data_Map, >1 - wait Data_Map before repeat request
} Data_map_send_table_t;

typedef struct
{
	Vch_Bridge_t *locking_brg;
	Vch_Bridge_t *locked_brg;
} Lock_list_t;

extern Virt_ch_t virt_ch_list[MAX_VIRT_CH];
extern U8_T global_virt_ch_num;
extern U8_T last_virt_ch_index;
extern U8_T sort_virt_ch_idx_list[MAX_VIRT_CH];
extern U8_T num_sorted_virt_chs;

extern Vch_Bridge_t bridge_list[MAX_VIRT_CH_BRIDGE];
extern Reserv_elem_t reserv_elem_list[MAX_RESERVATION_ENTRY];

extern U8_T lock_list_size;
extern Lock_list_t lock_list[MAX_NUM_LOCK_ENTRIES];


extern uint8_t service_msg_queue[QUEUE_BUF_SZ(SERV_MSG_QUEUE_SZ)];

#if !defined (__OMNET__)        // OMNET Compiler
//!!!CONVERTER_START_FIELD_->

U8_T get_new_virt_ch_num();

U8_T create_virt_ch(Virt_ch_t *virt_ch, uint16_t dest_addr, Traff_Priority_t priority, Traff_Type_t tc_type, TraffDirectionType direction);

void create_virt_ch_from_service_pkt(Virt_ch_t *virt_ch, Pkt_Service *srv_pkt);

U8_T find_free_pos_for_virt_ch();

Virt_ch_t* add_virt_ch(Virt_ch_t *virt_ch);

void add_to_sort_virt_ch_list(Virt_ch_t *new_virt_ch, U8_T new_vch_index);

Virt_ch_t* get_virt_ch_by_traffic_par(uint16_t dest_addr, uint16_t src_addr, Traff_Priority_t priority, Traff_Type_t tc_type);

Virt_ch_t* get_virt_ch_by_ID(uint16_t src_addr, U8_T virt_ch_num);

U8_T get_virt_ch_index(Virt_ch_t *virt_ch);

void delete_virt_ch(Virt_ch_t *virt_ch);

void delete_from_sort_virt_ch_list(U8_T del_vch_index);

void delete_direct_virt_ch_for_sta(uint16_t addr);

void delete_all_virt_ch_for_sta(uint16_t addr);

Vch_Readiness_t* get_readiness(Virt_ch_t *virt_ch, Vch_Bridge_t *bridge);

void set_readiness(Vch_Readiness_t *readiness, Vch_Ready_Func_t state, Vch_Ready_State_t ready, uint8_t response, uint8_t repeat_cntr);

Vch_Ready_State_t get_ready_state(Virt_ch_t *virt_ch, Vch_Bridge_t *bridge);

void prepare_virt_ch_readiness(Virt_ch_t *virt_ch, Vch_Bridge_t *bridge);

void plan_sending_non_ready_msg(Virt_ch_t *virt_ch);

U8_T check_curr_re_availability(Virt_ch_t *virt_ch, U8_T direction);

U8_T alloc_bridge();

Vch_Bridge_t* create_bridge(uint16_t dest, uint16_t src, U8_T da_infosrc, U8_T sa_infosrc);

void add_bridge_to_ptr_chain(Vch_Bridge_t **ptr_head_bridge, Vch_Bridge_t *bridge, Virt_ch_t *virt_ch);

void set_function_to_bridge(Vch_Bridge_t *bridge, Virt_ch_func_t vch_function);

void set_function_to_bridge_by_force(Vch_Bridge_t *bridge, Virt_ch_func_t vch_function);

Vch_Bridge_t* get_tx_bridge(Virt_ch_t *virt_ch);

Vch_Bridge_t* get_rx_bridge(Virt_ch_t *virt_ch);

U8_T get_bridge_index(Vch_Bridge_t *bridge);

uint16_t get_addr_to_next_node(Virt_ch_t *virt_ch);

uint16_t get_addr_to_prev_node(Virt_ch_t *virt_ch);

U8_T check_closing_of_bridges(Virt_ch_t *virt_ch);

void prepare_bridge_for_closing(Vch_Bridge_t *bridge);

void delete_bridge(Vch_Bridge_t **ptr_head_bridge, Vch_Bridge_t *bridge);

void delete_bridges(Vch_Bridge_t **ptr_head_bridge);

void add_lock_state_to_bridges(Vch_Bridge_t *locking_brg, Vch_Bridge_t *locked_brg);

void remove_lock_state_from_locking_bridge(Vch_Bridge_t *bridge);

void remove_lock_state_from_locked_bridge(Vch_Bridge_t *bridge);

U8_T alloc_reserv_entry();

Reserv_elem_t* create_reserv_entry(TraffDirectionType direction, uint8_t freq_ch, uint8_t num_tr_blocks, uint8_t offset, ModemRate_t rate, Reserv_elem_type_t type);

void add_reserv_entries_to_ptr_chain(Reserv_elem_t **ptr_head_elem, Reserv_elem_t *elem, Vch_Bridge_t *bridge);

Reserv_elem_t* copy_reserv_elem(Reserv_elem_t *elem, Reserv_elem_type_t type);

Reserv_elem_t* find_reserv_elem_by_position(Reserv_elem_t *head_elem, Reserv_elem_t *res_elem);

U8_T compare_reserv_entries(Reserv_elem_t *head_elem, Reserv_elem_t *head_elem2);

U8_T calc_num_reserv_entries(Virt_ch_t *virt_ch);

void release_reserv_entries(Reserv_elem_t *head_elem);

void release_all_reserv_entries(Vch_Bridge_t *bridge);

void delete_task_from_reserv_entry(Reserv_elem_t *reserv_elem);

void delete_task_from_reserv_entries(Reserv_elem_t *head_elem);

void delete_reserv_entry_from_ptr_chain(Reserv_elem_t **ptr_head_elem, Reserv_elem_t *elem);

void delete_reserv_entries(Reserv_elem_t **ptr_head_elem);

void delete_all_reserv_entries_from_bridge(Vch_Bridge_t *bridge);

#if (CHECK_VALID_RESERV_ENTRIES)
void check_leak_of_reserv_entries();

void check_readiness_without_curr_re();
#endif

//!!!CONVERTER_END_FIELD_<-
#endif

#endif // #ifndef __MAC_VIRT_CHANNEL_H
