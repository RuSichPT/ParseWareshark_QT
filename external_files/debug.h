
//debug.h//

//Define to prevent recursive inclusion
#ifndef __DEBUG_H
#define __DEBUG_H

#include "types.h"


#define DEBUG_ASM_BKPT					1

#if defined (__OMNET__)        // OMNET Compiler
#define DEBUG_MEMCPY                    0
#else
#define DEBUG_MEMCPY                    1
#endif// OMNET Compiler

#define DEBUG_RNDIS						1
#define DEBUG_PBUFS						1
#define DEBUG_TASK_MNGR					1

#define SNIFFER_MODE					0
#if SNIFFER_MODE
#define USE_LWIP
#endif

#define USE_TFTP						1
#define USE_LOGG						1

#if USE_TFTP
#define ERR_STRING_SIZE                 (96)

extern uint8_t ERROR_String[ERR_STRING_SIZE];

typedef union
{
	uint32_t U;
	struct
	{
		uint32_t debug_RX :1;
		uint32_t debug_speed_test :1;
		uint32_t debug_state :1;
		uint32_t debug_DATA_ANALYSIS :1;

		uint32_t debug_SIP_RX :1;
		uint32_t debug_SIP_TX :1;

		uint32_t debug_USB_MODEM_MODE :1;

//		uint32_t debug_BR_TcReq :1;
//		uint32_t debug_BR_TcBack :1;
//		uint32_t debug_BR_CtrlReq :1;
//
//		uint32_t debug_CVS_CTRL_Msg :1;
//		uint32_t debug_PreCVS_TcResp :1;
//		uint32_t debug_CVS_TcResp :1;
//		uint32_t debug_Vch_ctrl :1;
		uint32_t debug_Proc_PKT_ERR :1;
//		uint32_t debug_NEED_CP_CHG :1;

//		uint32_t debug_Traff :1;

	};
} Debug_commands_t;


#define DEBUG_SPEED_TEST_MODE			1


#endif // USE_TFTP

#if DEBUG_MEMCPY
#define MEMCPY                memcpy_dbg
#else
#define MEMCPY                memcpy
#endif

extern void show_error(const char* function, int line); //#include "Periphery/display.h"

#define ASM_NOP               asm("nop")
#if defined (__GNUC__) && !defined (__OMNET__)
#define ERROR_DEBUG_BKPT      show_error(__FUNCTION__, __LINE__);asm("bkpt")
#elif defined (__OMNET__)

#else
#define ERROR_DEBUG_BKPT      show_error(__FUNCTION__, __LINE__);asm("bkpt#0")
#endif

#if defined (__OMNET__)        // OMNET Compiler
#define F_CLASS MacLayer::
#define add_task(x)	x()
#else
  #define F_CLASS
#endif

#if defined (__ICCARM__)
#define PACKED          __packed
#define OPTIMIZE_FOR_SPEED  //В IAR есть #pragma optimize=speed, но использовать её можно только для понижения уровня оптимизации (как удобно, да?) //Примечание: если Вы используете директиву #pragma optimize, чтобы указать уровень оптимизации выше, чем это указано в опциях компилятора (или в настройках проекта или модуля), то директива pragma будет игнорирована.
#elif defined (__OMNET__)        // OMNET Compiler
#define PACKED          __attribute__ ((__packed__))
#define OPTIMIZE_FOR_SPEED
#elif defined (__GNUC__)
#define PACKED          __attribute__((__packed__))
#define OPTIMIZE_FOR_SPEED			__attribute__((optimize("-Ofast")))
#endif//#if defined (__ICCARM__)

#if DEBUG_MEMCPY
void *memcpy_dbg(void * _D, const void * _S, SIZE_T _N);
#endif

#endif // #ifndef __DEBUG_H
