//MAC_debug.h//

//Define to prevent recursive inclusion
#ifndef __MAC_DEBUG_H
#define __MAC_DEBUG_H

#include "external_files/debug.h"


#define DEBUG                            1

#if DEBUG

#define DEBUG_RESTART_ON                1
#define AUTO_SPEED_ENABLE               1
#define AUTO_POWER_ENABLE               1
#define AUTO_REPLY_ARP_ICMP				1

#define DEBUG_WITH_ETH_SEND             1
#define DEBUG_VCH_LIST             		1

#if defined (__ICCARM__)
#define PACKED          __packed
#elif defined (__ATOLLIC__)
#define PACKED          __attribute__((__packed__))
#elif defined (__OMNET__)        // OMNET Compiler
#define PACKED          __attribute__ ((__packed__))
#endif//#if defined (__ICCARM__)

#if defined (_MSC_VER)          // MSVC++

#define ERROR_DEBUG_BKPT      _asm int 3  //#include <windows.h> DebugBreak()
#elif defined (__OMNET__)        // OMNET Compiler
#define ASM_NOP               asm("nop")

#define ERROR_DEBUG_BKPT      printf("\n\n<!> RUNTIME ERROR: ERROR_DEBUG_BKPT:\n<!> %s\n<!> %s; line: %d\n<!> %s, addr = %d\n<!> time = %f\n", __FILE__, __FUNCTION__, __LINE__, ((uint8_t*)(&NodeName) + 16), mac.addr, (simTime().raw()) /1000000000000.0f ); fflush(stdout); DEBUG_TRAP // см дефайн DEBUG_TRAP в platmisc.h //asm("int $3\n")  // MinGW or Cygwin: debug interrupt with GNU syntax
//FunctionWithBreakPoint()

#define DEBUG_ASM_BKPT                  1

#define MEMCPY                          memcpy

#define DEBUG_TASK_MNGR                 1

#define WIRESHARK_EXPORT                1
#define WIRESHARK_EXPORT_BS_50          0

#define CHECK_VALID_RESERV_ENTRIES		1

#elif defined (__ICCARM__) || defined (__ATOLLIC__)       // IAR Compiler

#define DEBUG_ADD_RX_DELAY    0
#define DEBUG_ADD_TX_DELAY    0
#define RXTX_DELAY              (50*PRECISION_US_TO_NS) //us

#define DEBUG_ASM_BKPT                  1
#define DEBUG_GPIO_LED_EN               1

#define DEBUG_TASK_MNGR                 1

#define TC_OWER_RNDIS                   1

#define WIRESHARK_EXPORT                0
#define WIRESHARK_EXPORT_BS_50          0

#if SNIFFER_MODE
#define USE_LWIP
#endif

#if DEBUG_WITH_ETH_SEND || debug_with_wireless_send
#define DEBUG_SPEED_TEST_MODE           1
typedef struct
{
	//  uint8_t         Offset  :4;
	uint8_t cycle;
	uint8_t slot;
//  uint8_t         SuperSlot    :4;
//  uint8_t         Frame   :4;
} SNIFFER_Pkt_TimeTsk;
#endif

#elif defined (__TI_CCS__)      // TI CCS Compiler
#define ASM_NOP               asm(" nop")
#define ERROR_DEBUG_BKPT      Debug_Str_Send(__FUNCTION__, strlen(__FUNCTION__), EN_DEBUG_STATE);Debug_Str_Send(ERROR_String, user_sprintf_int(ERROR_String,"%", __LINE__), EN_DEBUG_STATE);asm(" SWBP 0")//;while (1){}
//asm(" NOP\n .word 0x10000000")
//asm(" .long 0x1001e000");

#define DEBUG_ASM_BKPT                  1

#define DEBUG_TASK_MNGR                 1

#elif defined (__MINGW32__) || defined (__MINGW64__)

#else //defined (__ICCARM__) || defined (__ATOLLIC__)
#error
#endif

#define OMNET_SNR_DB_OFFSET             0 // сколько дБ прибавлять к уровню сигнала в симуляторе при передаче значения в SLEVEL
#define BAD_RECEIVE_SNR                 (-120)



#if DEBUG_TASK_MNGR
extern uint8_t LastTaskID;
#endif

#else //DEBUG
#define DEBUG_RESTART_ON                1
#define AUTO_SPEED_ENABLE               1
#define AUTO_POWER_ENABLE               1

#define DEBUG_WITH_ETH_SEND             1
#endif

#if DEBUG_WITH_ETH_SEND

#define UCOS_DEBUG_CMD                  0x0100

typedef enum
{
	EN_DEBUG_RX = 0,
	EN_DEBUG_SPEED_TEST,
	EN_DEBUG_STATE,
	EN_DEBUG_DATA_ANALYSIS,

	EN_DEBUG_BR_TC_REQ,
	EN_DEBUG_BR_TC_RESET,
	EN_DEBUG_BR_TCBACK,
	EN_DEBUG_BR_CTRLREQ,

	EN_DEBUG_CVS_CTRL_MSG,
	EN_DEBUG_PRECVS_TCRESP,
	EN_DEBUG_CVS_TCRESP,
	EN_DEBUG_VCH_CTRL,
	EN_DEBUG_SHS10_MAP_RESET,
	EN_DEBUG_SHS10_PRIORITY_SEND,
	EN_DEBUG_SHS10_PRIORITY_RESET,
	EN_DEBUG_NEED_CP_CHG,

	EN_DEBUG_SLOT_DISTRIBUTION,
	EN_DEBUG_BCN_DISTRIBUTION,

	EN_DEBUG_TRAFF,
} Debug_commands_enum;

#define ERR_STRING_SIZE                 (96)
#define SLOT_DISTR_BUFFER_SIZE          (257)

extern uint8_t ERROR_String[ERR_STRING_SIZE];
extern Debug_commands_t Debug_commands;
extern uint32_t bad_TC_cntr;


#if defined (__ICCARM__) || defined (__ATOLLIC__)      // IAR Compiler
extern uint8_t Debug_HW_ADDR[];
extern uint16_t Debug_PORT;
extern uint32_t Debug_IP_ADDR;
#endif

#endif//DEBUG_WITH_ETH_SEND

#if !defined (__OMNET__)        // OMNET Compiler
//!!!CONVERTER_START_FIELD_->

#if defined (__OMNET__)        // OMNET Compiler

#if WIRESHARK_EXPORT
void shs50_ucos_pkt_to_log(uint8_t *data, uint32_t len);
#endif

char* output_BuffOwner(BuffOwner_typedef owner);

char* output_ModemType(ModemType_t type);

int16_t Pow_to_Snr(double pwr);

char* output_pkt_type(PktType type);

char* output_traffic_direction(TraffDirectionType dir);

char* output_traffic_type(Traff_Type_t type);

char* output_virt_ch_function(Virt_ch_func_t func);

char* output_task_operation(Task_Operation_t operation);

char* output_virt_ch_state(Vch_State_t state);

void output_reserv_enties(Reserv_elem_t *head_elem);

void output_virt_ch_list();

void outputStatusInfo();

void outputAddressTables();

void print_bcn_distr(uint8_t *distr);

void output_freqs();

#elif defined (__ICCARM__) || defined (__ATOLLIC__)      // IAR Compiler

#if DEBUG_WITH_ETH_SEND
void debug_MODEM_RX_DEBUG(uint8_t *pdat, uint16_t len);
#endif

#endif

void Debug_Str_Send(uint8_t *buf, uint16_t len, Debug_commands_enum en_cmd);

char* outputState(uint8_t state);

uint16_t gen_reserv_enties_state(void *_iter_elem, uint8_t *pdata);

void output_virt_ch_list_in_parsing(void *_type_Hdr);

void output_virt_ch_list_uart(void **_virt_ch, U8_T num_vch);

uint16_t write_virt_ch_to_pbuff(uint8_t *pdata, void *_virt_ch);

//!!!CONVERTER_END_FIELD_<-
#endif

#endif // #ifndef __MAC_DEBUG_H
