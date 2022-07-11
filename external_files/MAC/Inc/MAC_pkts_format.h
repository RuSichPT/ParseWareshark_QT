
//MAC_pkts_format.h//

//Define to prevent recursive inclusion
#ifndef __MAC_PKTS_FORMAT_H
#define __MAC_PKTS_FORMAT_H

#include "external_files/MAC/Inc/MAC_debug.h"
#include "external_files/MAC/Inc/MAC_Max_STA_Num.h"	// MAX_RADIO_CONNECTIONS
#include "external_files/MAC/Inc/MAC_Timers.h"			// Net_time_t
#include "external_files/eth.h"						// eth_addr


#define SOFT_ADDR_BRDCST        (0xFFFFu)
#define SOFT_ADDR_EMPTY         0

typedef enum SIZE_OF_ENUM_UINT8
{
	PKT_TYPE_BEACON				= 1,	// Brdcst
	PKT_TYPE_RTS				= 2,	// Brdcst
	PKT_TYPE_CTS				= 3,	// Brdcst
	PKT_TYPE_ACK				= 4,	// Brdcst
	PKT_TYPE_NACK				= 5,	// Brdcst
	PKT_TYPE_NACK_R				= 6,	// Brdcst // NACK Response
	PKT_TYPE_VCH_READINESS		= 7,	// Brdcst
	PKT_TYPE_DATA				= 8,	// Uncst
	PKT_TYPE_DATA_TIMEMARKED	= 9,	// Uncst
	PKT_TYPE_REQUEST			= 10,	// Uncst/Mltcst/Brdcst
	PKT_TYPE_DATA_MAP			= 11,	// Brdcst
	PKT_TYPE_DATA_MAP_REQUEST	= 12,	// Brdcst
	PKT_TYPE_CONNECT_REQUEST	= 13,	// Brdcst
	PKT_TYPE_TIME_CORRECTION	= 14,	// Brdcst
	PKT_TYPE_RREQ				= 15,	// запрос на маршрут Brdcst
	PKT_TYPE_RREP				= 16,	// ответ на запрос Uncst
	PKT_TYPE_RREQ_MAC_IP		= 17,	// RREQ c mac ip
	PKT_TYPE_RREP_MAC_IP		= 18,	// RREP c mac ip
	PKT_TYPE_RERR				= 19,	// ошибка маршрута Uncst/Brdcst
	PKT_TYPE_MAC_IP_REQ			= 20,	// запрос mac ip
	PKT_TYPE_MAC_IP_REP			= 21,	// ответ mac ip
	PKT_TYPE_NET_MAP			= 22,	// список сетевых адресов
	PKT_TYPE_CONTROL_NUMBER		= 23,	// пакет только с номером пакета для контроля пакетов

//	PKT_TYPE_DATA_LITE          = 6,    // Uncst/Mltcst/Brdcst

// 	PKT_TYPE_NEED_DISCONNECT			= 12,   //Brdcst
//	PKT_TYPE_REBUILD_REQ_INITIATIVE 	= 13,	//Brdcst
//	PKT_TYPE_REBUILD_REQ_RETRANSMISSION = 14,	//Brdcst
//	PKT_TYPE_TEST						= 15,	//Brdcst
	PKT_TYPE_MAX_VALUE			= 255,
} PACKED PktType;

typedef enum SIZE_OF_ENUM_UINT16
{
	REQ_TYPE_NEED_DATA_MAP	= 0,
	REQ_TYPE_DATA_MAP_END	= 1,
	REQ_TYPE_NEED_NET_MAP	= 2,
	REQ_TYPE_FFFF			= 0xFFFFu,
} PACKED RequestType;

/////////////BUSY TIME TABLE
typedef enum SIZE_OF_ENUM_UINT8
{
	TYPE_REALTIME = 0,
	TYPE_1 = 1,
	TYPE_2 = 2,
	TYPE_3 = 3
} PACKED TraffType;

typedef enum SIZE_OF_ENUM_UINT8
{
	PRIORITY_0 = 0,
	PRIORITY_1 = 1,
	PRIORITY_2 = 2,
	PRIORITY_3 = 3
} PACKED TraffPriority;

typedef enum SIZE_OF_ENUM_UINT8
{
	REASON_TRAFF_END 		= 0,
	REASON_ERROR_RESERVING	= 1,
	REASON_REDUCING			= 2,
	REASON_TRAFF_END_REP	= 3,
} PACKED ClosingReason_t;

typedef enum SIZE_OF_ENUM_UINT8
{
	RESULT_DISSENT 		= 0,	// не согласны
	RESULT_CONSENT 		= 1,	// согласны
	RESULT_2			= 2
} PACKED ReservingResult_t;

typedef enum SIZE_OF_ENUM_UINT8
{
	TYPE_SERVICE = 0, //Служебные данные, BEACON, RTS, CTS, ACK, NACK
	TYPE_DATA = 1
} PACKED BTT_TraffType;


#define PKT_ADDR_TYPE           uint16_t
#define PKT_LEN_TYPE            uint16_t

#define PKT_TYPE_TYPE           PktType
#define PKT_LENTONEXT_TYPE      uint8_t

#define PKT_CRC_TYPE            uint16_t

#pragma pack(push, 1)
typedef struct
{
	PKT_ADDR_TYPE DestAddr;
	PKT_ADDR_TYPE SrcAddr;
	PKT_LEN_TYPE PktLen;
	PKT_CRC_TYPE crc;
	PKT_TYPE_TYPE Data_type;
	PKT_LENTONEXT_TYPE Data_LenToNext;
} Pkt_Hdr;

typedef struct
{
	PKT_TYPE_TYPE Type;
	PKT_LENTONEXT_TYPE LenToNext;
	uint8_t pdata[];
} Pkt_type_Hdr;
#pragma pack(pop)

#define PKT_HDR_LEN (sizeof(Pkt_Hdr) - sizeof(Pkt_type_Hdr) )

#define MODEM_RX_BUFF_SIZE      (MAX_BYTES_PER_TX + PKT_HDR_LEN)// + PKT_ID_SZ)//(GET_LEN_MAX_SPD(MAX_SLOTS_PER_DATA_TX) + PKT_HDR_LEN)

#define PKT_ADDR_LEN            sizeof(PKT_ADDR_TYPE)
#define PKT_LEN_LEN             sizeof(PKT_LEN_TYPE)
#define PKT_TYPE_LEN            sizeof(PKT_TYPE_TYPE)
#define PKT_LENTONEXT_LEN       sizeof(PKT_LENTONEXT_TYPE)

#define ADDRESSBOOK_PAGE_SZ     (sizeof(AddressTable_t)*MAX_RADIO_CONNECTIONS)


typedef enum
{
	SNR_READY = 1,
	SNR_NOT_READY = 0,
} snr_state_t;

typedef enum
{
	RATE_READY = 1,
	RATE_NOT_READY = 0,
} tx_rate_state_t;



// Максимальное количество используемых частот
#define BR_FREQ_NUM             (16u)
#define NUM_FREQ_CH             (BR_FREQ_NUM)

#pragma pack(push, 1)

typedef struct
{
	uint16_t addr :16;
} AS_State_t;

typedef struct
{
	uint8_t possible_rx_rate :3;
	uint8_t next_cycle_tx_rate :3;
} AS_Speed_t;

typedef enum SIZE_OF_ENUM_UINT8
{
	VSV_NONE	= 0, //нет связи, в этом месте в таблице должен отсутствовать адрес
	VSV_CNCT	= 1, //абонент в состоянии подключения
	VSV_RSV		= 2, //есть радиосвязь
	VSV_PSV		= 3, //есть проводная связь
	VSV_RPSV	= 4, //есть и проводная и радиосвязь одновременно
	VSV_NCNCT	= 5, //нет связи, это САС для соседа, в состояниии подключения
	VSV_NRSV	= 6, //нет связи, это САС для соседа, с которой у соседа радиосвязь
	VSV_NPSV	= 7, //нет связи, это САС для соседа, с которой у соседа провод
	VSV_NRPSV	= 8, //нет связи, это САС для соседа, с которой у соседа и радио и провод
} PACKED VSV_t;

#define VSV_BITS                    4
#define VSV_MSK                     ((1 << VSV_BITS) - 1)
//#define GET_VSV(vsv, index)         ((VSV_t) (((vsv) >> ((index) * VSV_BITS)) & VSV_MSK) )
//#define SET_VSV(dest, index, vsv)   ((dest) = ( ((dest) & (~ (VSV_MSK << ((index) * VSV_BITS))) ) | ((vsv) << ((index) * VSV_BITS)) ))

#define BEACON_RECEIVED           1
#define BEACON_NOT_RECEIVED       0

typedef struct
{
	uint16_t				addr;
	uint8_t					vsv :VSV_BITS;
	uint8_t					bcn_rx :1; //BEACON_RECEIVED / BEACON_NOT_RECEIVED
} AddressTable_t;

#define FRQ_ARR_SZ                      (BR_FREQ_NUM >> 1)
#define FRQ_BITS                        4
#define FRQ_MSK                         ((1 << FRQ_BITS) - 1)
#define GET_FRQ(FRQ, index)             ((FRQ[(index) >> 1] >> (FRQ_BITS * ((index) & 0x1))) & FRQ_MSK)
#define SET_FRQ(dest, index, val)       (dest[(index) >> 1] = ( (dest[(index) >> 1] & (~ (FRQ_MSK << (FRQ_BITS * ((index) & 0x1)))) ) | ((val) << (FRQ_BITS * ((index) & 0x1))) ) )

typedef struct
{
	uint8_t					max_as;				// необязательное поле, для wireshark
	uint8_t					curr_frame;
	uint8_t					curr_cycle_size;
	uint8_t					next_cycle_size;
	uint8_t					next_bcn_cycle;		// неясно, так ли необходимо это поле, только если бикон должен быть послан в следующем цикле и во фрейме, значение которого превышает значение фрейма при приеме
	uint8_t					next_bcn_frame;
} Network_Par_t;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	Network_Par_t			NET_PAR;
	AS_State_t				AS[MAX_RADIO_CONNECTIONS];
	AS_Speed_t				SPD[MAX_RADIO_CONNECTIONS];
	uint8_t					VSV[(MAX_RADIO_CONNECTIONS + 1) >> 1]; // вид связи для каждого абонента, адрес которого указан в адресной таблице
	uint8_t					FRQ[FRQ_ARR_SZ];
	int8_t					SNR_AS[MAX_RADIO_CONNECTIONS];
	uint8_t					last_heard_pkt_number[MAX_RADIO_CONNECTIONS]; // список из последних услышанных порядковых номеров пакетов(широковещательных)
	int8_t					self_clock_deviation;
} Pkt_Beacon;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	int16_t					time_offset[MAX_RADIO_CONNECTIONS];
} Pkt_Time_Correction_t;
#pragma pack(pop)

#define PKT_BEACON_LEN sizeof(Pkt_Beacon)

#pragma pack(push, 1)
typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint16_t				init_addr;			// адрес инициатора перетягивания к проводной сети
	int16_t					slot_offset;		// временной сдвиг между сетями
	uint16_t				FHSS_freqs;			// набор частот в чужой сети
	int32_t					FHSS_step_Offset;			// сдвиг шага ППРЧ
	int8_t					SYN_FHSS_CycleOffset_Offset;// сдвиг номера цикла в котором запрашивали ППРЧ для пакетов синхронизации
	int8_t					cycle_offset;		// коррекция номера цикла в группе группового цикла
	uint8_t					pseudo_host_BI;
	AS_State_t				AS[MAX_RADIO_CONNECTIONS];
	uint32_t				VSV;				// вид связи для каждого абонента, адрес которого указан в адресной таблице
} Pkt_Rebuild_Req;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union
{
	uint32_t FHSS_step;
} Timestamp_TypeDef;

#define SYN_FOR_NET_UNION   1
#define SYN_FOR_SYNC        0

typedef struct
{
	uint16_t				FreqSetNum	:10;	//Номер комплекта частот (номер частоты) - используется модемом для отсеивания пакетов синхронизации
	uint16_t				vks			:1;		// Номер сети (ВКС1/ВКС2) - используется модемом для отсеивания пакетов синхронизации
	uint16_t				CycleOffset	:4;		//сколько циклов назад генерировалась последовательность частот для пакетов синхронизации
	uint16_t				reserv		:1;
	PKT_ADDR_TYPE			SA;					//source addr
	Timestamp_TypeDef		timestamp;

	uint16_t				Freqs	:16; //Используемые частоты
	uint16_t				sc		:2;  //у нас используется как признак пакета синхронизации для объединения сетей  // признак сокращенного цикла у вадима
	uint16_t				Blank	:7;  // если sc == 1 используется как номер ИНР
	uint16_t				res		:3;
	uint16_t				crc_1	:4;
	uint16_t				crc_2	:12;
	uint16_t						:4;
} Pkt_SynchrMsg;
#define PKT_SYNCHR_LEN sizeof(Pkt_SynchrMsg)
typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint16_t				HostAddr;
	uint8_t					BeaconInterval;
} Pkt_ConnectRequest;

#pragma pack(pop)


// *** RTS/CTS/ACK/NACK ***
#pragma pack(push, 1)
typedef enum SIZE_OF_ENUM_UINT8
{
	DIRECTION_TYPE_RESERV	= 0x00,
	DIRECTION_TYPE_RX		= 0x01,
	DIRECTION_TYPE_TX		= 0x02,
	DIRECTION_TYPE_DUPLEX	= 0x03,
} PACKED TraffDirectionType;

//typedef enum SIZE_OF_ENUM_UINT8
//{
//	RESERV_COMMON = 0,
//	RESERV_REQUEST = 1
//} PACKED Reservation_type_t;

typedef enum SIZE_OF_ENUM_UINT8
{
	RE_CURRENT				,	// текущая ЗоР
	RE_PLANNED 				,	// планируемая ЗоР
	RE_CLOSING 				,	// вспомогательная ЗоР, для закрытия временного участка
	RE_RESERVED
} PACKED Reserv_elem_type_t;

typedef struct
{
	uint8_t 				num_tr_blocks;	// кол-во транспортных блоков // если кол-во ТБ равно 0, значит ЗоР пустая
	uint8_t 				offset;			// смещение

	uint8_t 				freq_ch			:4; // частотный канал
	TraffDirectionType		direction		:2; // направление (00-Резерв, 01-Прм, 02-Прд, 03-дуплекс )
	Reserv_elem_type_t		type			:2;	// тип ЗоР: текущая, планируемая, вспомогательная (+) или (-)

	uint8_t 				rate			:2; // скорость (0,1,2...)
	uint8_t					reserved		:6;
} Reservation_entry_t;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint16_t				dest_addr;		// адрес конечного абонента
	uint16_t				src_addr;		// адрес инициатора обмена
	uint8_t					virt_ch_num;	// номер виртуального канала
	uint8_t					session_id;		// номер сессии организации или изменения участка ВК
	uint16_t				sec_addr;		// вспогательный адрес (адрес получателя/отправителя данных в участке ВК)
	uint8_t					pkt_number;		// порядковый номер пакета
	union
	{
		struct
		{
			uint8_t				traffic_type	:4;		// тип трафика
			TraffDirectionType	direction		:2;		// направление (00-Резерв, 01-Прм, 02-Прд, 03-дуплекс )
			uint8_t				priority		:2;		// приоритет
		};
		ReservingResult_t		reserv_result	:4;		// результат резервирования
		ClosingReason_t			closing_reason	:4;		// причина закрытия вирт канала
	};
	Reservation_entry_t			res_entry[];	// запись о резервировании
} Pkt_Service;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint8_t					pkt_number;		// порядковый номер пакета
} Pkt_control_number;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	RequestType 			request_code;	// код запроса
} Pkt_Request;

// ************ Routing ************
typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint8_t					join_flag				:1;
	uint8_t					repair_flag				:1;
	uint8_t					gratuitous_RREP_flag	:1;
	uint8_t					dest_only_flag			:1;
	uint8_t					unknown_seq_num_flag	:1;
	uint8_t					reserved				:3;
	uint8_t					hop_count;
	uint32_t				rreq_id;
	uint16_t				dest_sta_addr;
	uint32_t				dest_seq_num;
	uint16_t				src_sta_addr;
	uint32_t				src_seq_num;
	int8_t					src_min_snr;
} Pkt_RREQ;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint8_t					repair_flag				:1;
	uint8_t					ack_required_flag		:1;
	uint8_t					reserved				:1;
	uint8_t					prefixSize				:5;
	uint8_t					hop_count;
	uint16_t				dest_sta_addr;
	uint32_t				dest_seq_num;
	int8_t					dest_min_snr;
	uint16_t				src_sta_addr;
} Pkt_RREP;

typedef struct
{
	Pkt_RREQ				rreq_pkt;
	eth_addr				src_mac_addr;
	uint32_t				src_ip_addr;
} Pkt_RREQ_mac_ip;

typedef struct
{
	Pkt_RREP				rrep_pkt;
	eth_addr				dest_mac_addr;
	uint32_t				dest_ip_addr;
} Pkt_RREP_mac_ip;

#pragma pack(push, 1)
typedef enum
{
	INVALID			,
	VALID			,
} PACKED Valid_flag_t;

typedef struct
{
	uint16_t				sta_addr;
	uint32_t				seq_num;
	Valid_flag_t			valid_flag;
} Unreachable_node_t;
#pragma pack(pop)

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint8_t					node_count;
	uint8_t 				no_delete				:1;
	uint8_t					reserved				:7;
	Unreachable_node_t		unreachable_nodes[];
} Pkt_RERR;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint16_t				src_sta_addr;
	eth_addr				src_mac_addr;
	uint32_t				src_ip_addr;
	uint16_t				dest_sta_addr;
} Pkt_mac_ip_req;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint16_t				dest_sta_addr;
	eth_addr				dest_mac_addr;
	uint32_t				dest_ip_addr;
	uint16_t				src_sta_addr;
} Pkt_mac_ip_rep;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint8_t					size;
	uint16_t				sta_addr[];
} Pkt_net_map;

typedef struct
{
	uint16_t				sta;
	uint32_t				ip;
	eth_addr				mac;
} Net_addr_t;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint8_t					size;
	Net_addr_t				net_addr[];
} Pkt_net_map_eth;

typedef struct
{
	PKT_TYPE_TYPE 			Type;
	PKT_LENTONEXT_TYPE		LenToNext;
	uint16_t 				src_addr;				// адрес инициатора обмена
	uint8_t					virt_ch_num;			// номер виртуального канала
	uint8_t					ready			: 4;	// 1 - участок ВК готов, 0 - участок ВК не готов
	uint8_t					response		: 4;	// 1 - на это сообщ-е отвечать не нужно
	uint16_t 				br_dest_addr;			// адрес принимающего узла участка ВК
	uint16_t 				br_src_addr;			// адрес передающего узла участка ВК
} Pkt_Vch_Readiness;

// ************ Routing ************

#pragma pack(pop)

#define PKT_DATA_LENTONEXT_TYPE      uint16_t

#pragma pack(push, 1)
typedef enum SIZE_OF_ENUM_UINT8
{
	FRAGMENT_FIRST_AND_LAST	= 0,
	FRAGMENT_FIRST_NOT_LAST	= 1,
	FRAGMENT_NOT_LAST		= 2,
	FRAGMENT_LAST			= 3
} PACKED FragmentEnd;

typedef struct
{
	uint8_t					Num	:4; //номер фрагмента внутри пакета
	uint8_t					Pkt	:2; //номер текущего пакета, меняется только при фрагментировании, используется для идентификации того, что текущий принятый фрагмент это продолжение предыдущего пакета, а не какойто фрагмент уже нового пакета
	FragmentEnd				End	:2; //признак того, что фрагмент последний
} FragmentType;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_DATA_LENTONEXT_TYPE	LenToNext;
	uint16_t				src_addr;
	uint8_t					virt_ch_num;
	FragmentType			fragment;
	uint8_t					pData[];
} Pkt_DATA;

typedef struct
{
	PKT_TYPE_TYPE			Type;
	PKT_DATA_LENTONEXT_TYPE	LenToNext;
	uint16_t				src_addr;
	uint8_t					virt_ch_num;
	FragmentType			fragment;
	Net_time_t				time_stamp;
	uint8_t					pData[];
} Pkt_DATA_TIMEMARKED;
#pragma pack(pop)

#define PKT_DATA_WITH_HDR_SZ                     (sizeof(Pkt_DATA) + PKT_HDR_LEN)
#define PKT_DATA_TYPE_LTN_SZ                     (sizeof(PKT_TYPE_TYPE) + sizeof(PKT_DATA_LENTONEXT_TYPE))
#define PKT_DATA_ONLY_HDR_SZ                     (sizeof(Pkt_DATA) - PKT_DATA_TYPE_LTN_SZ)


#endif // #ifndef __MAC_PKTS_FORMAT_H
