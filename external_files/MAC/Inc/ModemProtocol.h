//ModemProtocol.h//

//Define to prevent recursive inclusion
#ifndef __MODEMPROTOCOL_H
#define __MODEMPROTOCOL_H

#include "external_files/MAC/Inc/MAC_debug.h"

//#define MODEM_SBS
//#define MODEM_PANZYR_SM
//#define MODEM_CC1101
#define MODEM_CC1312

#if defined(MODEM_SBS)

#define GetLen_PRD1_SYNC(time_us)					((uint32_t)((((time_us) == 2000)*14) +  (((time_us) != 2000)* ((((time_us)*(0.045)) - 15) / 6.0)  )))
#define GetLen_PRD1(time_us)						((uint32_t)(((((time_us) - 16)*(0.04496875)) - 35) / 6.0) )
#define GetLen_PRD2(time_us)						((uint32_t)(((((time_us) - 16)*(0.04496875)) - 26) / 3.0) )
#define GetLen_PRD3(time_us)						((uint32_t)((( (time_us)      *(0.045))      - 27) * (2/3.0)) )
#define GetLen_PRD4(time_us)						((uint32_t)((( (time_us)      *(0.045))      - 27) * (4/3.0)) )
#define GetLen_PRD5(time_us)						((uint32_t)((( (time_us)      *(0.045))      - 27) * (8/3.0)) )
#define GetLen_PRD6(time_us)						((uint32_t)((( (time_us)      *(0.045))      - 27) * (16/3.0)) )

#define GET_MIN_SPEED_LEN(slots)					GetLen_PRD1(((slots) * US_IN_IPS*PP_FRQ_PER_SLOT))
#define GET_SLOTS_MIN_SPEED							GetNumSlots_PRD1
#define GET_SLOTS_MIN_SPEED_SYNC					GetNumSlots_PRD1_SYNC

#define GetNumSlots_PRD1_SYNC(len, t_inr) 			(uint32_t)(((len<=14)*1) + ((len>14)*(((((((len)*6) + 15) / 0.045) / ((t_inr) * 1.0))) + 1)))
#define GetNumSlots_PRD1(len, t_inr) 				(uint32_t)(((((((len)*6) + 35) / 0.04496875) + 16) / ((t_inr) * 1.0)) + 1)

#define	MODEM_SPEED_CNT								6//сколько скоросетй умеет модем
// Параметры КСС
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_KSS_RATE_PRD1 = (0x0000u),
	UCOS_KSS_RATE_PRD2 = (0x0001u),
	UCOS_KSS_RATE_PRD3 = (0x0002u),
	UCOS_KSS_RATE_PRD4 = (0x0003u),
	UCOS_KSS_RATE_PRD5 = (0x0004u),
	UCOS_KSS_RATE_PRD6 = (0x0005u),
} ModemRate_t;

#define MAX_BYTES_PER_TX							(4000)
#define MAX_SLOTS_PER_DATA_TX   					(40)

#define MIN_SNR_TO_USE_DIRECT_ROUTE					-7 //dB если на соседа с/ш ниже этого значения, значит нужно пытаться искать на него непрямые маршруты
#define MAX_SNR_PRD1_UP								0 //dB порог на повышение скорости начиная с которого работаем на второй скорости

#define MIN_SPEED               					UCOS_KSS_RATE_PRD1
#define MAX_SPEED               					UCOS_KSS_RATE_PRD5
#define MAX_SPEED_TYPE          					UCOS_KSS_TYPE_NORM

//#define SPEED_TO_SEPARATE_HDR   					UCOS_KSS_RATE_PRD2 //скорость начиная с которой нужно заголовок посылать отдельным пакетом
//#define HIGH_SPEED_FOR_FEC      					(UCOS_KSS_RATE_PRD4)
//#define LOW_SPEED_SLOTS_PER_FEC_BLOCK   			(4)
//#define HIGH_SPEED_SLOTS_PER_FEC_BLOCK  			(2)

#define CTRL_TX_SPEED           					UCOS_KSS_RATE_PRD2

#define SPEED_THR_TO_ALLOW_FRQ						(MAX_SPEED - NORM_PWR_TH_RATE)//(2u) //на сколько ступеней может снижаться скорость при возвращении частоты в сравнении от текущей широковещательной скорости на передаче

#define GET_LEN_MAX_SPD								GET_LEN_PRD5

// уровень мощности в текущем ТБ
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_KSS_POWER_LOWLOW = (0x0000u),	// малый  1 Вт
	UCOS_KSS_POWER_LOW = (0x0001u),	// пониженный 20 Вт;
	UCOS_KSS_POWER_NORM = (0x0002u),	// номинальный 150 Вт
} ModemPower_t;

#define MIN_POWER_IDX								UCOS_KSS_POWER_LOWLOW
#define MAX_POWER_IDX								UCOS_KSS_POWER_NORM

#define MODEM_SYNCHR_DELAY                          (1677*PRECISION_US_TO_NS) // через сколько после приема выдается CURRENTSTATUS
#define MODEM_SLOT_DELAY                            2 //через сколько ИНР после приема команды от нас модем ее исполнит( ((MODEM_TASK_DELAY - 1) / TIME_FOR_SLOT) + 1 ) // должно быть < SLOTS_ON_SUPERSLOT //количество транспортных слотов, которые можно успеть принять/передать во время кодирования/декодирования принятого

#if defined (__OMNET__)       // __OMNET__ Compiler
#define MODEM_SYNCHR_SD_DELAY                       (MODEM_SYNCHR_DELAY)//x100nsec // время, которое выдает модем для пакета UCOS_KSS_TYPE_SINHR если нет временной задержки
#define MODEM_CON_REQ_SD_DELAY                      (14900 - MODEM_TIME_PROTECT_SLOT_INTERV)//x100nsec // время, которое выдает модем для пакета UCOS_KSS_TYPE_TIME_MEAS если нет временной задержки

#define MODEM_SYNCHR_TIME_CORRECTION_ERROR			(-396) 	//x100nsec //величина подобрана так, чтобы при приёме синхра вычисленное значение задержки совпадало с временем распространения сигнала в канале
#define MODEM_CON_REQ_TIME_CORRECTION_ERROR			(655) 	//x100nsec //величина подобрана так, чтобы при приёме запроса на подключение вычисленное значение задержки совпадало с временем распространения сигнала в канале
#define MODEM_SYNCHR_DELAY_DEVIATION 				(-103)//(137) // (PRECISION_US_TO_NS учтена) в OMNET передача пакета синхр-ции занимает на столько мкс больше (т.к. chanel_len вычисляется с точностью до байта в SENDING)

#else
#define MODEM_SYNCHR_SD_DELAY                       (942*PRECISION_US_TO_NS) // время, которое выдает модем для пакета UCOS_KSS_TYPE_SINHR если нет временной задержки
#define MODEM_CON_REQ_SD_DELAY                      (942*PRECISION_US_TO_NS) // время, которое выдает модем для пакета UCOS_KSS_TYPE_TIME_MEAS если нет временной задержки

#define MODEM_SYNCHR_TIME_CORRECTION_ERROR			(-3*PRECISION_US_TO_NS) 	//x100nsec
#define MODEM_CON_REQ_TIME_CORRECTION_ERROR			(-3*PRECISION_US_TO_NS) 	//x100nsec
#endif//#if defined (__OMNET__)       // __OMNET__ Compiler

#define PP_FRQ_PER_SLOT         					4
#elif defined(MODEM_PANZYR_SM)

#define GetLen_PRD1_SYNC(time_us)					((uint32_t)((((time_us) == 2000)*14) +  (((time_us) != 2000)* ((((time_us)*(0.045)) - 15) / 6.0)  )))
#define GetLen_PRD1(time_us)						((uint32_t)(((((time_us) - 16)*(0.04496875)) - 35) / 6.0) )
#define GetLen_PRD2(time_us)						((uint32_t)(((((time_us) - 16)*(0.04496875)) - 26) / 3.0) )
#define GetLen_PRD3(time_us)						((uint32_t)((( (time_us)      *(0.045))      - 27) * (2/3.0)) )
#define GetLen_PRD4(time_us)						((uint32_t)((( (time_us)      *(0.045))      - 27) * (4/3.0)) )
#define GetLen_PRD5(time_us)						((uint32_t)((( (time_us)      *(0.045))      - 27) * (8/3.0)) )

#define GET_MIN_SPEED_LEN(slots)					GetLen_PRD1(((slots) * US_IN_IPS*PP_FRQ_PER_SLOT))
#define GET_SLOTS_MIN_SPEED							GetNumSlots_PRD1
#define GET_SLOTS_MIN_SPEED_SYNC					GetNumSlots_PRD1_SYNC

#define GetNumSlots_PRD1_SYNC(len, t_inr) 			(uint32_t)(((len<=14)*1) + ((len>14)*(((((((len)*6) + 15) / 0.045) / ((t_inr) * 1.0))) + 1)))
#define GetNumSlots_PRD1(len, t_inr) 				(uint32_t)(((((((len)*6) + 35) / 0.04496875) + 16) / ((t_inr) * 1.0)) + 1)

#define	MODEM_SPEED_CNT								5//сколько скоросетй умеет модем
// Параметры КСС
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_KSS_RATE_PRD1 = (0x0000u),
	UCOS_KSS_RATE_PRD2 = (0x0001u),
	UCOS_KSS_RATE_PRD3 = (0x0002u),
	UCOS_KSS_RATE_PRD4 = (0x0003u),
	UCOS_KSS_RATE_PRD5 = (0x0004u),
	UCOS_KSS_RATE_PRD6 = (0x0005u),
} ModemRate_t;

#define MAX_BYTES_PER_TX							(4000)
#define MAX_SLOTS_PER_DATA_TX   					(40)

#define MIN_SNR_TO_USE_DIRECT_ROUTE					-7 //dB если на соседа с/ш ниже этого значения, значит нужно пытаться искать на него непрямые маршруты
#define MAX_SNR_PRD1_UP								0 //dB порог на повышение скорости начиная с которого работаем на второй скорости

#define MIN_SPEED               					UCOS_KSS_RATE_PRD1
#define MAX_SPEED               					UCOS_KSS_RATE_PRD5
#define MAX_SPEED_TYPE          					UCOS_KSS_TYPE_NORM

//#define SPEED_TO_SEPARATE_HDR   					UCOS_KSS_RATE_PRD2 //скорость начиная с которой нужно заголовок посылать отдельным пакетом
//#define HIGH_SPEED_FOR_FEC      					(UCOS_KSS_RATE_PRD4)
//#define LOW_SPEED_SLOTS_PER_FEC_BLOCK   			(4)
//#define HIGH_SPEED_SLOTS_PER_FEC_BLOCK  			(2)

#define CTRL_TX_SPEED           					UCOS_KSS_RATE_PRD2

#define SPEED_THR_TO_ALLOW_FRQ						(MAX_SPEED - NORM_PWR_TH_RATE)//(2u) //на сколько ступеней может снижаться скорость при возвращении частоты в сравнении от текущей широковещательной скорости на передаче

#define GET_LEN_MAX_SPD								GET_LEN_PRD5

// уровень мощности в текущем ТБ
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_KSS_POWER_LOW = (0x0000u),	// пониженный
	UCOS_KSS_POWER_NORM = (0x0001u),	// номинальный
} ModemPower_t;

#define MIN_POWER_IDX								UCOS_KSS_POWER_LOW
#define MAX_POWER_IDX								UCOS_KSS_POWER_NORM

#define MODEM_SYNCHR_DELAY                          (1677*PRECISION_US_TO_NS) // через сколько после приема выдается CURRENTSTATUS
#define MODEM_SLOT_DELAY                            2 //через сколько ИНР после приема команды от нас модем ее исполнит( ((MODEM_TASK_DELAY - 1) / TIME_FOR_SLOT) + 1 ) // должно быть < SLOTS_ON_SUPERSLOT //количество транспортных слотов, которые можно успеть принять/передать во время кодирования/декодирования принятого

#if defined (__OMNET__)       // __OMNET__ Compiler
#define MODEM_SYNCHR_SD_DELAY                       (MODEM_SYNCHR_DELAY)//x100nsec // время, которое выдает модем для пакета UCOS_KSS_TYPE_SINHR если нет временной задержки
#define MODEM_CON_REQ_SD_DELAY                      (14900 - MODEM_TIME_PROTECT_SLOT_INTERV)//x100nsec // время, которое выдает модем для пакета UCOS_KSS_TYPE_TIME_MEAS если нет временной задержки

#define MODEM_SYNCHR_TIME_CORRECTION_ERROR			(-396) 	//x100nsec //величина подобрана так, чтобы при приёме синхра вычисленное значение задержки совпадало с временем распространения сигнала в канале
#define MODEM_CON_REQ_TIME_CORRECTION_ERROR			(655) 	//x100nsec //величина подобрана так, чтобы при приёме запроса на подключение вычисленное значение задержки совпадало с временем распространения сигнала в канале
#define MODEM_SYNCHR_DELAY_DEVIATION 				(-103)//(137) // (PRECISION_US_TO_NS учтена) в OMNET передача пакета синхр-ции занимает на столько мкс больше (т.к. chanel_len вычисляется с точностью до байта в SENDING)

#else
#define MODEM_SYNCHR_SD_DELAY                       (942*PRECISION_US_TO_NS) // время, которое выдает модем для пакета UCOS_KSS_TYPE_SINHR если нет временной задержки
#define MODEM_CON_REQ_SD_DELAY                      (942*PRECISION_US_TO_NS) // время, которое выдает модем для пакета UCOS_KSS_TYPE_TIME_MEAS если нет временной задержки

#define MODEM_SYNCHR_TIME_CORRECTION_ERROR			(-3*PRECISION_US_TO_NS) 	//x100nsec
#define MODEM_CON_REQ_TIME_CORRECTION_ERROR			(-3*PRECISION_US_TO_NS) 	//x100nsec
#endif//#if defined (__OMNET__)       // __OMNET__ Compiler

#define PP_FRQ_PER_SLOT         					4
#elif defined(MODEM_CC1101)
#define PREAMBLE_BYTES								12
#define SYNCWORD_BYTES								4
#define LEN_BYTES			    					2
#define CRC_BYTES			    					2
#define GetLen_PRD1(Time)       					( (Time) < 1200 ? 0 : (uint32_t)( (((((Time) - 600)*0.061) - PREAMBLE_BYTES - SYNCWORD_BYTES) / 2) - LEN_BYTES - CRC_BYTES ) )
#define GetLen_PRD2(Time)       					( (Time) < 1200 ? 0 : (uint32_t)( (((((Time) - 600)*0.061) - PREAMBLE_BYTES - SYNCWORD_BYTES) ) - LEN_BYTES - CRC_BYTES ) )

#define GET_MIN_SPEED_LEN(slots)					GetLen_PRD1(((slots) * US_IN_IPS*PP_FRQ_PER_SLOT))

#define	MODEM_SPEED_CNT								2//сколько скоросетй умеет модем

// Параметры КСС
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_KSS_RATE_PRD1 = (0x0000u),
	UCOS_KSS_RATE_PRD2 = (0x0001u),
} ModemRate_t;

#define MIN_SNR_TO_USE_DIRECT_ROUTE					-7 //dB если на соседа с/ш ниже этого значения, значит нужно пытаться искать на него непрямые маршруты
#define MAX_SNR_PRD1_UP								0 //dB порог на повышение скорости начиная с которого работаем на второй скорости

#define MIN_SPEED               					UCOS_KSS_RATE_PRD1
#define MAX_SPEED               					UCOS_KSS_RATE_PRD2
#define MAX_SPEED_TYPE          					UCOS_KSS_TYPE_NORM


#define CTRL_TX_SPEED           					MIN_SPEED


#define SPEED_THR_TO_ALLOW_FRQ						(MAX_SPEED - NORM_PWR_TH_RATE)//(2u) //на сколько ступеней может снижаться скорость при возвращении частоты в сравнении от текущей широковещательной скорости на передаче

//#define GET_LEN_MAX_SPD(slots)					GetLen_PRD2((slots)*US_IN_IPS*PP_FRQ_PER_SLOT)

#define MODEM_SYNCHR_DELAY                          (1677*PRECISION_US_TO_NS) // через сколько после приема выдается CURRENTSTATUS
#define MODEM_SLOT_DELAY                            0 //через сколько ИНР после приема команды от нас модем ее исполнит( ((MODEM_TASK_DELAY - 1) / TIME_FOR_SLOT) + 1 ) // должно быть < SLOTS_ON_SUPERSLOT //количество транспортных слотов, которые можно успеть принять/передать во время кодирования/декодирования принятого

#define PP_FRQ_PER_SLOT         					1
#elif defined(MODEM_CC1312)
#define PREAMBLE_BYTES								4
#define SYNCWORD_BYTES								4
#define LEN_BYTES									2
#define CRC_BYTES									2
#define GetLen_PRD1(Time)							( (Time) < 1200 ? 0 : (uint32_t)( (((((Time) - 600)*0.061) - PREAMBLE_BYTES - SYNCWORD_BYTES) / 2) - LEN_BYTES - CRC_BYTES ) )
#define GetLen_PRD2(Time)							( (Time) < 1200 ? 0 : (uint32_t)( (((((Time) - 600)*0.061) - PREAMBLE_BYTES - SYNCWORD_BYTES) ) - LEN_BYTES - CRC_BYTES ) )
#define GetLen_PRD3(Time)							( (Time) < 1200 ? 0 : (uint32_t)( (((((Time) - 700)*0.061) - PREAMBLE_BYTES - SYNCWORD_BYTES) * 2) - LEN_BYTES - CRC_BYTES ) )

#define GET_MIN_SPEED_LEN(slots)					GetLen_PRD1(((slots) * US_IN_IPS*PP_FRQ_PER_SLOT))
#define GET_SLOTS_MIN_SPEED							GetNumSlots_PRD1
#define GET_SLOTS_MIN_SPEED_SYNC					GetNumSlots_PRD1

#define GetNumSlots_PRD1(len, t_inr)				(uint32_t)( ( ( (len) * 2000 + (2 * (LEN_BYTES + CRC_BYTES) + PREAMBLE_BYTES + SYNCWORD_BYTES) * 1000) / 61.0 + 600) / ((t_inr) * 1.0) + 1)
#define GetNumSlots_PRD2(len, t_inr)				(uint32_t)( ( ( (len) * 1000 + (LEN_BYTES + CRC_BYTES + PREAMBLE_BYTES + SYNCWORD_BYTES) * 1000) / 61.0 + 600) / ((t_inr) * 1.0) + 1)
#define GetNumSlots_PRD3(len, t_inr)				(uint32_t)( ( ( (len) * 500 + (((LEN_BYTES + CRC_BYTES) >> 1) + PREAMBLE_BYTES + SYNCWORD_BYTES) * 1000) / 61.0 + 700) / ((t_inr) * 1.0) + 1)

#define	MODEM_SPEED_CNT								3//сколько скоростей умеет модем

// Параметры КСС
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_KSS_RATE_PRD1 = (0x0000u),
	UCOS_KSS_RATE_PRD2 = (0x0001u),
	UCOS_KSS_RATE_PRD3 = (0x0002u),
} ModemRate_t;

#define MAX_BYTES_PER_TX							(4047 - sizeof(uint16_t))//GET_LEN_MAX_SPD(MAX_SLOTS_PER_DATA_TX)
#define MAX_SLOTS_PER_DATA_TX   					(70)//calc_num_TB_from_bytes(MAX_BYTES_PER_TX, MAX_SPEED)//ModemCommand_t->count :6 бит -закостылили там 7 бит // maximum = SLOTS_PER_ALL_COMMON_DATA_SLOTS

#define MIN_SNR_TO_USE_DIRECT_ROUTE					-85 //dB если на соседа с/ш ниже этого значения, значит нужно пытаться искать на него непрямые маршруты
#define MAX_SNR_PRD1_UP								-65 //dB порог на повышение скорости начиная с которого работаем на второй скорости

#define MIN_SPEED               					UCOS_KSS_RATE_PRD1
#define MAX_SPEED               					UCOS_KSS_RATE_PRD3
#define MAX_SPEED_TYPE          					UCOS_KSS_TYPE_NORM


#define CTRL_TX_SPEED           					MIN_SPEED


#define SPEED_THR_TO_ALLOW_FRQ						(MAX_SPEED - NORM_PWR_TH_RATE)//(2u) //на сколько ступеней может снижаться скорость при возвращении частоты в сравнении от текущей широковещательной скорости на передаче

//#define GET_LEN_MAX_SPD(slots)					GetLen_PRD3((slots)*US_IN_IPS*PP_FRQ_PER_SLOT)

// уровень мощности в текущем ТБ
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_POWER_m11dbm = (0x0000u),
	UCOS_POWER_m8dbm ,
	UCOS_POWER_m6dbm ,
	UCOS_POWER_0dbm  ,
	UCOS_POWER_1dbm  ,
	UCOS_POWER_2dbm  ,
	UCOS_POWER_3dbm  ,
	UCOS_POWER_4dbm  ,
	UCOS_POWER_5dbm  ,
	UCOS_POWER_6dbm  ,
	UCOS_POWER_7dbm  ,
	UCOS_POWER_8dbm  ,
	UCOS_POWER_9dbm  ,
	UCOS_POWER_10dbm ,
	UCOS_POWER_11dbm ,
	UCOS_POWER_12dbm ,
	UCOS_POWER_13dbm ,
} ModemPower_t;

#define MIN_POWER_IDX								UCOS_POWER_m11dbm
#define MAX_POWER_IDX								UCOS_POWER_13dbm

#define MODEM_SYNCHR_DELAY                          (1677*PRECISION_US_TO_NS) // через сколько после приема выдается CURRENTSTATUS
#define MODEM_SLOT_DELAY                            0 //через сколько ИНР после приема команды от нас модем ее исполнит( ((MODEM_TASK_DELAY - 1) / TIME_FOR_SLOT) + 1 ) // должно быть < SLOTS_ON_SUPERSLOT //количество транспортных слотов, которые можно успеть принять/передать во время кодирования/декодирования принятого

#if defined (__OMNET__)       // __OMNET__ Compiler
#define MODEM_SYNCHR_SD_DELAY                       (MODEM_SYNCHR_DELAY)//x100nsec // время, которое выдает модем для пакета UCOS_KSS_TYPE_SINHR если нет временной задержки
#define MODEM_CON_REQ_SD_DELAY                      (14900 - MODEM_TIME_PROTECT_SLOT_INTERV)//x100nsec // время, которое выдает модем для пакета UCOS_KSS_TYPE_TIME_MEAS если нет временной задержки

#define MODEM_SYNCHR_TIME_CORRECTION_ERROR			(-396) 	//x100nsec //величина подобрана так, чтобы при приёме синхра вычисленное значение задержки совпадало с временем распространения сигнала в канале
#define MODEM_CON_REQ_TIME_CORRECTION_ERROR			(655) 	//x100nsec //величина подобрана так, чтобы при приёме запроса на подключение вычисленное значение задержки совпадало с временем распространения сигнала в канале
#define MODEM_SYNCHR_DELAY_DEVIATION 				(-103)//(137) // (PRECISION_US_TO_NS учтена) в OMNET передача пакета синхр-ции занимает на столько мкс больше (т.к. chanel_len вычисляется с точностью до байта в SENDING)
#else
#define MODEM_SYNCHR_SD_DELAY                       (942*PRECISION_US_TO_NS) // время, которое выдает модем для пакета UCOS_KSS_TYPE_SINHR если нет временной задержки
#define MODEM_CON_REQ_SD_DELAY                      (942*PRECISION_US_TO_NS) // время, которое выдает модем для пакета UCOS_KSS_TYPE_TIME_MEAS если нет временной задержки

#define MODEM_SYNCHR_TIME_CORRECTION_ERROR			(-3*PRECISION_US_TO_NS) 	//x100nsec
#define MODEM_CON_REQ_TIME_CORRECTION_ERROR			(-3*PRECISION_US_TO_NS) 	//x100nsec
#endif//#if defined (__OMNET__)       // __OMNET__ Compiler

#define PP_FRQ_PER_SLOT         1
#else
#error
#endif//#elif defined(MODEM_CC1101)


#define SYNHR_TX_SPEED          					MIN_SPEED
#define BEACON_TX_SPEED         					MIN_SPEED
#define CONN_REQ_TX_SPEED       					MIN_SPEED
#define VOC_TX_SPEED            					MIN_SPEED

#define ALLOW_ALL_FREQS         					0xFFFF // нет выколотых частот

#define FRQ_COMMON_CH           					10//MAX_RADIO_CONNECTIONS
#define FRQ_LISTENING           					1//((mac.own.addr + num_restarts) % BR_FREQ_NUM) //1 //для уменьшения вероятности коллизий при подключении прослушиваемая частота зависит от адреса
#define FRQ_FF_INDEX            					0//индекс частоты ФЧ для массива номеров частот из которого будем брать частоту заданную для ФЧ


#if !defined (__TI_CCS__)      // TI CCS Compiler

// Команды управления УЦОС
typedef enum //SIZE_OF_ENUM_UINT16
{
	UCOS_CMD_RESPONSE = (0x8000u),
	UCOS_CMD_BR = (0x0002u), // рабочий режим
	UCOS_CMD_RO = (0x0004u), // режим ожидания
	UCOS_CMD_FSET = (0x0005u), // FSET
	UCOS_CMD_TUNE = (0x0006u),
	UCOS_CMD_CURRENT_STATUS = (0x0009u), //(0x8009u)
	UCOS_CMD_RXDATA = (0x0011u), //(0x8011u)
	UCOS_CMD_SLEVEL = (0x0012u), //(0x8012u)
	UCOS_CMD_TXDATA = (0x0013u),
	UCOS_CMD_CHECK_A = (0x0021u),
	UCOS_CMD_RCHECK_R = (0x0022u),
	UCOS_CMD_RCHECK_T = (0x0023u),
	UCOS_CMD_CHECK_TEST = (0x0024u),
	UCOS_CMD_FCS_RANGE = (0x0061u),
	UCOS_CMD_SUM_CHECK = (0x0063u),
	UCOS_CMD_UUIO_SUMCHECK = (0x0064u), //(0x8064u)
	UCOS_CMD_CODE = (0x0065u),
	UCOS_CMD_UNKNOWN = (0x0071u), //(0x8071u)
	UCOS_CMD_END = (0xFFFFu)
} PACKED Modem_CMD_Names;

// направление канала передачи
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_KSS_TR_PRM = (0x0000u),	// прием
	UCOS_KSS_TR_PRD = (0x0001u),	// передача
	UCOS_KSS_TR_SCAN = (0x0002u),	// сканирование
	UCOS_KSS_TR_NONE = (0x0003u),	// ничего не делать
} ModemDir_t;

// Тип слота
typedef enum SIZE_OF_ENUM_UINT32
{
	UCOS_KSS_TYPE_NORM = (0x0000u),	// обычный              // ПЧ2000 !!
	UCOS_KSS_TYPE_SINHR = (0x0001u),	// синхронизации        // ПЧ500  !!
	UCOS_KSS_TYPE_TIME_MEAS = (0x0002u),	//спец пакет для измерения времени рапространения сигнала
} ModemType_t;

// Определяет необходимо ли подстраивать ИНР при синхронизации
typedef enum SIZE_OF_ENUM_UINT32
{
  UCOS_KSS_NEED_NEW_SYN     = (0u), // обязательно успешно принимать синхронизацию перед приемом данных
  UCOS_KSS_KEEP_OLD_SYN     = (1u), // можно принимать данные без синхронизации
} ModemSyncKeep_t;


#define POWER_TABLE_SZ      						(MAX_POWER_IDX + 1)

// Общий формат команды на УЦОС
typedef struct
{
	uint16_t cmd;
	uint16_t len;   // длина сообщения в байтах
	uint16_t data[];
} ucos_packet_t;

#endif//#if !defined (__TI_CCS__)      // TI CCS Compiler


#pragma pack(push, 1)
typedef union
{
  struct
  {
    int16_t delay;//на сколько сместиться при синхронизации, если UCOS_KSS_TYPE_SINHR;Используется для удержания синхронизации в НП.
#if defined(MODEM_SBS) || defined(MODEM_PANZYR_SM)
    uint16_t AGC;//Используется для удержания синхронизации в НП.
#endif
  };
#if defined(MODEM_SBS) || defined(MODEM_PANZYR_SM)
  uint32_t U;
#else
  uint16_t U;
#endif
} TmShft_AGC_t;

typedef struct
{
	ModemRate_t rate :3;   // скорость передачи
	ModemDir_t tr :3;   // направление канала передачи
#if defined(MODEM_SBS)
	ModemType_t type :1;   // тип
	uint32_t rsrvd1 :1;
	uint32_t count :6;   // количество ТБ в команде //у Вадима - количество слотов в данном ТБ
	uint32_t TU :6; //у Вадима -  Определяет номер транспортного блока  (Принимает значения 0 – 39.) ТБ – группа последовательно расположенных слотов с одинаковыми параметрами. Предназначается для приема/передачиданных от/к одному абоненту.
	ModemPower_t power :2;   // уровень мощности//в битность должна влазить MAX_POWER
	ModemSyncKeep_t SyncKeep:1;//Флаг удержания синхронизации //Если 0, то обязательно успешно принимать синхронизацию перед приемом данных, иначе можно принимать данные без синхронизации.
	uint32_t rsrvd0 :9;
	TmShft_AGC_t          TmShft_AGC;
#elif defined(MODEM_PANZYR_SM)
	ModemType_t type :1;   // тип
	ModemPower_t power :1;   // уровень мощности//в битность должна влазить MAX_POWER
	uint32_t count :6;   // количество ТБ в команде //у Вадима - количество слотов в данном ТБ
	uint32_t TU :6; //у Вадима -  Определяет номер транспортного блока  (Принимает значения 0 – 39.) ТБ – группа последовательно расположенных слотов с одинаковыми параметрами. Предназначается для приема/передачиданных от/к одному абоненту.
	ModemSyncKeep_t SyncKeep:1;//Флаг удержания синхронизации //Если 0, то обязательно успешно принимать синхронизацию перед приемом данных, иначе можно принимать данные без синхронизации.
	uint32_t rsrvd0  :11;
	TmShft_AGC_t          TmShft_AGC;
#elif defined(MODEM_CC1101)
	ModemType_t type :2;   // тип
	ModemPower_t power :8;   // уровень мощности//в битность должна влазить MAX_POWER
	uint32_t count :8;   // количество ТБ в команде //у Вадима - количество слотов в данном ТБ
	uint32_t TU :6; //у Вадима -  Определяет номер транспортного блока  (Принимает значения 0 – 39.) ТБ – группа последовательно расположенных слотов с одинаковыми параметрами. Предназначается для приема/передачиданных от/к одному абоненту.
	uint32_t SNum :1;   // Номер синхропосылки при поиске синхронизации  0…14
	ModemTune_t Tune :1; // Определяет, необходимо ли подстраивать ИНР при приеме синхропосылки. 0 – без подстройки; 1 – с подстройкой; 2 – с “обратной” подстройкой.
	int16_t delay; //на сколько сместиться при синхронизации, если UCOS_KSS_TYPE_SINHR
#elif defined(MODEM_CC1312)
	ModemType_t type :2;   // тип
	ModemPower_t power :8;   // уровень мощности//в битность должна влазить MAX_POWER
	uint32_t count :8;   // количество ТБ в команде //у Вадима - количество слотов в данном ТБ
	uint32_t TU :6; //у Вадима -  Определяет номер транспортного блока  (Принимает значения 0 – 39.) ТБ – группа последовательно расположенных слотов с одинаковыми параметрами. Предназначается для приема/передачиданных от/к одному абоненту.
	ModemSyncKeep_t SyncKeep:1; // Определяет, необходимо ли подстраивать ИНР при приеме синхропосылки. 0 – без подстройки; 1 – с подстройкой; 2 – с “обратной” подстройкой.
	uint32_t rsrvd0 :1;
	TmShft_AGC_t          TmShft_AGC;
#else
#error
#endif//#elif defined(MODEM_CC1101)


} ModemCommand_t;

// Формат ответа на команду FSET
typedef struct
{
	uint16_t id;  // Порядковый номер команды.
	uint16_t Status; //0 – ОК;  1 – Неверный параметр команды;  2 – Отказ
	union
	{
		uint16_t Error; //Содержит данные об отказе составных частей
		struct
		{
			uint16_t SH :1;   //0 – СЧ ОК; 1 – Отказ
			uint16_t UM :1;   //0 – УМ ОК; 1 – Отказ
			uint16_t prg :1;   //0 – УМ ОК; 1 – Перегрев
			uint16_t F :1;   //0 – Ф ОК;  1 – Отказ
			uint16_t :12;
		};
	};
} Pkt_fset_status_t;
#pragma pack(pop)

#pragma pack(push, 2)
typedef struct
{
	uint16_t R;
	uint16_t N;
} ModemFRQ_Coef_TypeDef;

typedef union
{
	ModemFRQ_Coef_TypeDef FrqCoefArray[1];
	uint32_t FrqArray[1];
} Modem_FSET_FRQ_t;

// Общий формат команды на УЦОС
typedef struct
{
	Modem_CMD_Names cmd;
	uint16_t len;   // длина сообщения в байтах
} Modem_CMD_HDR_TypeDef;

// Формат команды FSET
typedef struct
{
	Modem_CMD_HDR_TypeDef HDR;
	uint16_t reserv;   //v7.0.18
	uint16_t prmn;   //v7.0.18 Применяемость  2 байта  Начало действия команды после приема (только для СтП): 0 – со следующего слота; 1 – через один слот.
	uint16_t id;                     // Идентификатор команды  // у Вадима - см  ucos_fset_id_t
	uint16_t num_freq;               // Номер комплекта частот
	uint16_t CommandWords_CNT;    //у Вадима -  num_tb;		// Количество транспортных блоков в команде, транспортный блок - это модемный блок с синхропосылкой
#if defined(MODEM_SBS)
	uint16_t circ;                   // Цикличность 0 – обычная работа с конфигурацией каждого слота;	1 – циклический повтор.
#elif defined(MODEM_PANZYR_SM)
	uint16_t vks;                    // Номер сети
#else
	uint16_t reserv_;
#endif
	ModemCommand_t CommandsArray[];                    // Командное слово слота
	/*  union{                                                      //и рабочие частоты
	 ModemFRQ_Coef_TypeDef FrqCoefArray[1];
	 uint32_t              FrqArray[1];
	 };
	 */
} Modem_FSET_Command_TypeDef;

#define MODEM_FSET_HDR_SIZE     (sizeof(Modem_FSET_Command_TypeDef) - sizeof(Modem_CMD_HDR_TypeDef))
#define MODEM_FSET_MAX_SIZE     (1368 + sizeof(Modem_CMD_HDR_TypeDef)) // todo 15.01 v7.0.5 // old: #define MODEM_FSET_MAX_SIZE     (1288 + sizeof(Modem_CMD_HDR_TypeDef))

// Формат данных на передачу
typedef struct
{
	Modem_CMD_HDR_TypeDef HDR;
	uint16_t id;             // идентификатор
	uint16_t fset_id;        // идентификатор команды FSET
	uint16_t TU;
	uint16_t cmd_num;        // todo 15.01 v7.0.5 Порядковый номер команды для текущего ТБ // old:   uint16_t              Reserved;
	uint8_t pData[];
} Modem_TXDATA_Command_TypeDef;

#define MODEM_TXDATA_HDR_SIZE     (sizeof(Modem_TXDATA_Command_TypeDef) - sizeof(Modem_CMD_HDR_TypeDef))

// Формат данных на прием
typedef struct
{
	Modem_CMD_HDR_TypeDef HDR;
	uint16_t id;             // идентификатор
	uint16_t fset_id;        // идентификатор команды FSET
	uint16_t TU;
	uint16_t cmd_num;        // todo 15.01 v7.0.5 Порядковый номер команды для текущего ТБ // old:   uint16_t              Reserved;
	uint8_t pData[];
} Modem_RXDATA_Command_TypeDef;
#pragma pack(pop)

#define MODEM_RXDATA_HDR_SIZE   (sizeof(Modem_RXDATA_Command_TypeDef) - sizeof(Modem_CMD_HDR_TypeDef))

#define SNR_VAL_TYPE    int8_t
#define MIN_SNR         ((SNR_VAL_TYPE)(1 << ( (8 * sizeof(SNR_VAL_TYPE)) - 1)))//(-128)//(0x80)//
#define MAX_SNR         ((SNR_VAL_TYPE)( ~ MIN_SNR))//(127)//(0x7F)//

#if !defined (__TI_CCS__)      // TI CCS Compiler
#pragma pack(push, 1)
// Формат выдачи ОСШ на одну частоту
typedef struct
{
	uint16_t res;                   // v 7.0.7 //uint16_t N;			// коэффициент N частоты
	SNR_VAL_TYPE val;               // v 7.0.7 Знаковая величина. Определяет ОСШ с шагом 1 дБ.
	struct
	{                        // v 7.0.7 Тип помехи и глубина замирания
		uint8_t band :1;		// Флаг присутствия узкополосной помехи
		uint8_t impulse :1;	    // Флаг присутствия импульсной помехи
		uint8_t fading :1;		// Флаг присутствия замирания
		uint8_t depth :5;		// Глубина замирания
	} noise;
	uint16_t sur;					// Средний уровень сигнала (upd 7.0.4)
	uint16_t mur;					// Максимальный уровень сигнала (upd 7.0.4)
} ucos_snr_t;
#pragma pack(pop)
#endif

#pragma pack(push, 2)
// Формат сообщения с ОСШ
typedef struct
{
	Modem_CMD_HDR_TypeDef HDR;
	uint16_t id;             // идентификатор
	uint16_t fset_id;        // идентификатор команды FSET
	uint16_t tb;             // номер ТБ
	uint16_t cmd_num;        // todo 15.01 v7.0.5 Порядковый номер команды для текущего ТБ // old:     uint16_t              res;
	uint16_t distance;       // v7.0.7 Дальность связи в м. Только на ведущей станции.
	uint16_t res;
	ucos_snr_t snr[];
} Modem_SLEVEL_Command_TypeDef;

#define MODEM_SLEVEL_HDR_SIZE   (sizeof(Modem_SLEVEL_Command_TypeDef) - sizeof(Modem_CMD_HDR_TypeDef))
#pragma pack(pop)

#pragma pack(push, 1)
// Формат сообщения с синхропосылкой
typedef struct
{
	Modem_CMD_HDR_TypeDef HDR;
	uint16_t done;   // признак синхронизации                        // 0– нет синхронизации; 1– данные синхронизации приняты
	uint16_t fset_id;   //v7.03 0…65535  Номер соответствующей команды FSET (только для "старого парка")
	uint8_t sinhr_data[14];  // служебное сообщение канала синхронизации
} Modem_CURRENT_STATUS_Command_TypeDef;
#pragma pack(pop)

#define MODEM_CURRENT_STATUS_HDR_SIZE   (sizeof(Modem_CURRENT_STATUS_Command_TypeDef) - sizeof(Modem_CMD_HDR_TypeDef))

//#define MAX_SLEVEL_LEN          ( sizeof(Modem_SLEVEL_Command_TypeDef) + (sizeof(ucos_snr_t) * PP_FRQ_PER_SLOT * SLOTS_ON_SUPERSLOT) )

#define MODEM_SLEVEL_MAX_SIZE   ( sizeof(Modem_SLEVEL_Command_TypeDef) + (sizeof(ucos_snr_t) * PP_FRQ_PER_SLOT * MAX_SLOTS_PER_DATA_TX) )

#define MODEM_TXDATA_DATA_OFFSET	(sizeof(eth_hdr) + sizeof(ip_hdr) + sizeof(udp_t) + sizeof(Modem_TXDATA_Command_TypeDef))//отступ от ETH_FRAME, откуда начнутся полезные данные
#define MODEM_TXDATA_LIMIT		(ETH_FRAME_SIZE - MODEM_TXDATA_DATA_OFFSET)//максимальный размер полезных данных, которые можно отправить модему одной командой TXDATA

extern uint8_t FSET_Command_buff[MODEM_FSET_MAX_SIZE];
extern uint8_t SLEVEL_Command_buff[MODEM_SLEVEL_MAX_SIZE];
extern int16_t thresholds_rate_up[MODEM_SPEED_CNT-1];

#if !defined (__OMNET__)
//!!!CONVERTER_START_FIELD_->

uint16_t GetLen_fromRate(uint16_t Slots, ModemRate_t rate, ModemType_t type);

ModemRate_t calc_rate_down_for_snr(int16_t snr_val);

ModemRate_t calc_rate_up_for_snr(int16_t snr_val);

ubase_t calc_pwr_idx_from_snr(ubase_t curr_pwr_idx, int16_t snr, ModemRate_t speed);

int16_t power_idx_to_dbm(ubase_t pwr_idx);

//uint16_t calc_tc_size_for_rate(ModemRate_t rate);
//
//uint16_t calc_slots_to_TX(uint16_t Len, ModemRate_t rate, Modem_FSET_Command_TypeDef *Modem_FSET_Command);
//
//ubase_t calc_slots_good_FEC_block(uint16_t Len, ModemRate_t rate);
//
//void Modem_Proc_FSET_Status(Pkt_fset_status_t *FSET_status);

#if defined (__TI_CCS__)      // TI CCS Compiler
void Modem_SendCommand_FSET(Modem_FSET_Command_TypeDef *FSETcommand); //,ubase_t superslot_num);

void Modem_SendCommand_TXDATA(Modem_TXDATA_Command_TypeDef *TXDATACommand);

void Modem_SendCommand_NEWTIME(uint32_t new_time, TimerSetter_typedef timerSetter);

void Modem_RecalcRXDelay(int32_t TimeOffset);

#endif

//!!!CONVERTER_END_FIELD_<-
#endif

#endif // #ifndef __MODEMPROTOCOL_H
