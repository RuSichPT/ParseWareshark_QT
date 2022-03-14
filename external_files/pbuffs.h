//pbufs.h//

//Define to prevent recursive inclusion
#ifndef __PBUFFS_H
#define __PBUFFS_H

#include "eth.h"//для ETH_MTU
#include "debug.h"//для F_CLASS
#include "types.h"
#include "MAC/Inc/MAC_Timers.h"//Net_time_t
#include "MAC/Inc/MAC_pkts_format.h"//PKT_LEN_TYPE

#if defined (__OMNET__)        // OMNET Compiler
#define RXTX_BUFFERS            40                                      //всего буферов под входящие пакеты
#else
#define RXTX_BUFFERS            40                                      //всего буферов под входящие пакеты
#endif

#pragma pack(push, 1)
typedef struct
{
	PKT_LEN_TYPE PktLen;
	uint8_t is_fragment;//признак того, что пакет из delay buffer был скопирован не полностью в slot_buffer
	uint8_t pData[];
} packed_hdr_t;
#pragma pack(pop)

#define ETH_FRAME_SIZE			(ETH_MTU + SIZEOF_ETH_HDR)
#define BUFF_SIZE               (ETH_FRAME_SIZE + (sizeof(packed_hdr_t) *2))              //#include "eth.h" //ETH_MTU + SIZEOF_ETH_HDR
#define MAX_TX_QUEUE_SIZE       10                                      //сколько буферов держать пока занят канал передачи
#define QUEUE_SIZE              (sizeof(uint8_t) + MAX_TX_QUEUE_SIZE)   //sizeof(indexQueue_TypeDef->Size) + сколько буферов держать пока занят канал передачи

#pragma pack(push, 1)
typedef enum
{
	MAC_FROM_RADIO,
	MAC_TO_RADIO,
	FROM_HIGH_LEVEL,
	TO_HIGH_LEVEL,
	FROM_HIGH_LEVEL_TO_DELAY_BUFF,
	TO_SLOT_BUFF,
	FROM_PARSE_TO_DEFRAGMENTATION,
	FROM_RNDIS,
	TO_RNDIS,
	FOR_REPEAT_PKT,
	TO_DBG_VCH,
	TO_FREE_DISTR,
	TO_LOG,
} BuffOwner_typedef;

typedef enum
{
	P_PRIORITY_0 = 0,
	P_PRIORITY_1 = 1,
	P_PRIORITY_2 = 2,
	P_PRIORITY_3 = 3
} Traff_Priority_t;

#define MAX_TRAFF_PRIORITY		P_PRIORITY_0
#define MIN_TRAFF_PRIORITY		P_PRIORITY_3
#define NUM_TRAFF_PRIORITY		(MIN_TRAFF_PRIORITY + 1) // число элементов в enum!

typedef enum
{
	P_TYPE_REALTIME 	= 0,
	P_TYPE_TIMEMARKED	= 1,
	P_TYPE_ASYNC 		= 2,
	P_TYPE_SERVICE 		= 3,
} Traff_Type_t;

typedef struct
{
    Traff_Priority_t Priority :2;
    Traff_Type_t Type :4;
} Traff_Params_t;

#pragma pack(pop)

#define MAX_TRAFF_TYPE          P_TYPE_REALTIME
#define MIN_TRAFF_TYPE          P_TYPE_SERVICE
#define NUM_TRAFF_TYPE      	(MIN_TRAFF_TYPE + 1) // число элементов в enum!

#define SMALL_PKTS_SZ			(sizeof(eth_hdr) + sizeof(arp_message_t)) //пакеты передаются без резервирования канала (RTS/CTS) если их длина меньше или равна этому значению
//#define SMALL_PKTS_SZ			(sizeof(icmp_struct)) //пакеты передаются без резервирования канала (RTS/CTS) если их длина меньше или равна этому значению


typedef struct
{
	Net_time_t time_stamp;
	Traff_Params_t TraffParams;
} pbuffs_TX_to_CH_type;

typedef struct
{
	RX_time_full_t time_stamp;
	uint16_t id;
} pbuffs_RX_from_CH_type;

typedef struct
{
	uint8_t buff[BUFF_SIZE];
	uint16_t len;
	BuffOwner_typedef owner;

	union
	{
		pbuffs_TX_to_CH_type TX;
		pbuffs_RX_from_CH_type RX;
	};
} pbuffs_typedef;

#define QUEUE_TRUE_SZ(x)	((x) + 1)//т.к. очередь не заполняется если write_idx + 1 % maxQueueSize == read_idx
#define QUEUE_SIZE_T		uint8_t
#define QUEUE_BUF_SZ(x)			(sizeof(QUEUE_SIZE_T) + sizeof(QUEUE_SIZE_T) + (x))
#pragma pack(push, 1)
typedef struct
{
	QUEUE_SIZE_T write_idx;
	QUEUE_SIZE_T read_idx;
	QUEUE_SIZE_T IndexTable[];
} indexQueue_TypeDef;
#pragma pack(pop)

extern uint32_t (*pbuffs_Disable_Interrupts)(void);
extern void (*pbuffs_Enable_Interrupts)(uint32_t);

#if !defined (__OMNET__)  
extern pbuffs_typedef pbuffs_to_RXTX[];

//!!!CONVERTER_START_FIELD_->

void init_pbuffs_to_RXTX(uint32_t (*func_Disable_Interrupts)(void), void (*func_Enable_Interrupts)(uint32_t));

void free_all_pbuffs();

int32_t alloc_pbuff_to_RXTX(BuffOwner_typedef need_owner);

void free_pbuff(pbuffs_typedef *pbuff);

void free_pbuff_to_RXTX(U8_T pbuff_index, BuffOwner_typedef need_owner);

int32_t find_pbuffindex_from_pointer(uint8_t *pointer);

QUEUE_SIZE_T extract_pDataFromQueue(indexQueue_TypeDef *Queue, QUEUE_SIZE_T maxQueueSize);

QUEUE_SIZE_T push_pDataInQueue(QUEUE_SIZE_T index, indexQueue_TypeDef *Queue, QUEUE_SIZE_T maxQueueSize);

QUEUE_SIZE_T Check_QueueSize(const indexQueue_TypeDef *Queue, QUEUE_SIZE_T maxQueueSize);

//!!!CONVERTER_END_FIELD_<-
#endif

#endif // #ifndef __PBUFFS_H
