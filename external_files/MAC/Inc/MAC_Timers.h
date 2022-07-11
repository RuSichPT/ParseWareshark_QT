
//MAC_Timers.h//

//Define to prevent recursive inclusion
#ifndef __MAC_TIMERS_H
#define __MAC_TIMERS_H

#include "external_files/MAC/Inc/MAC_debug.h"

extern uint32_t INR;

typedef uint16_t Net_time_t;
#define NET_TIME_US_PER_TICK						(US_IN_IPS<<1)	//US_IN_IPS<<1 - сколько мксек приходится на единицу значения, возвращаемого Get_Net_Time()
#define NET_TIME_TICKS_PER_CYCLE					((TIME_WORK_PERIOD / PRECISION_US_TO_NS) / NET_TIME_US_PER_TICK)
#define MAX_TIME_STAMP								0xFFFF
#define MAX_TIME_CORRECTION							(MODEM_TIME_PROTECT_SLOT_INTERV * 2)    //абонентам со сдвигом больше этого не будут отвечать на запросы на подключение
#define MAX_SYNC_OFFSET_TO_SEND_TIME_CORRECTION		(MODEM_TIME_PROTECT_SLOT_INTERV / 5) 	//если вылезли за эту величину, нужно высылать TIME_CORRECTION
#define MAX_SYNC_INR_CORRECTION						(1) //на сколько максимум ИНРов нас можно подстроить, если мы в сети и у нас есть соединения (защита от приёма синхров от злоумышленников)
#define MAX_AS_RESYNC								(MODEM_TIME_PROTECT_SLOT_INTERV / 2)    //на сколько максимум абонент может переместиться по расстоянию за групповой цикл.

typedef int32_t RX_time_full_t;

#if !defined (__OMNET__)        // OMNET Compiler
//!!!CONVERTER_START_FIELD_->

uint16_t GetINR();

void SetINR(uint32_t inr);

Net_time_t Get_Net_Time();

//!!!CONVERTER_END_FIELD_<-
#endif

#endif // #ifndef __MAC_TIMERS_H
