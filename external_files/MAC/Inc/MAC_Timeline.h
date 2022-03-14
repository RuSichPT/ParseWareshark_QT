
//MAC_Timeline.h//

//Define to prevent recursive inclusion
#ifndef __MAC_TIMELINE_H
#define __MAC_TIMELINE_H

#include "MAC/Inc/MAC_debug.h"
#include "MAC/Inc/MAC_Max_STA_Num.h"	//MAX_RADIO_CONNECTIONS
#include "MAC/Inc/ModemProtocol.h"		//PP_FRQ_PER_SLOT, GetSlot_PRD1(), ..
#include "MAC/Inc/MAC_pkts_format.h"	//PKT_BEACON_LEN


// для FSET команд
#define ID_CYCLE_SHFT								9		//на сколько бит двигать номер цикла, или сколько бит из 16 на отображение номера слота в ID команд
#define ID_SLOT_MSK									((1 << ID_CYCLE_SHFT) - 1)
#define ID_CYCLE_MSK								((1 << (16 - ID_CYCLE_SHFT)) - 1)

#define ID_UNIQUE_CMD								SLOTS_PER_WORK_PERIOD//уникальный ИД, число, которое не может быть сгенерировано generate_cmd_ID()

#define BEACON_TB_NUM								(0)
#define DATA_TB_NUM									(0)

// ~ для SLOTS_PER_BEACON == 6, расположение CMN_INT(i) зависит от кол-ва фреймов в цикле
// времянка TX/RX: | 0(SYNC) | 4(RSRV) | 7(BCN) | ~13(IVA) | ~19(PRI_INT) | ~37(CMN_INT(0)) | (CMN_INT(i)) | 200
#define TX_SYNCHR_SLOT                       		(0)
#define TX_RESERV_SLOT                       		(4)

//#define ORDER_BEACON_IVA                       		// взаимное расположение BEACON и IVA на ВД: BEACON -> IVA(по-умолчанию) или IVA -> BEACON

#ifdef ORDER_BEACON_IVA
#define TX_BEACON_SLOT                       		(7)
#define IVA_SLOT                    				(TX_BEACON_SLOT + SLOTS_PER_BEACON)
#define PRIOR_TX_INTERVAL_SLOT						IVA_SLOT_I(NUM_IVA)
#endif

#ifndef ORDER_BEACON_IVA
// времянка TX/RX: | 0(SYNC) | 4(RSRV) | 7(IVA) | 13(BCN) | ~19(PRI_INT) | ~37(CMN_INT(0)) | (CMN_INT(i)) | 200
#define IVA_SLOT                    				(7)
#define TX_BEACON_SLOT                       		IVA_SLOT_I(NUM_IVA)
#define PRIOR_TX_INTERVAL_SLOT						(TX_BEACON_SLOT + SLOTS_PER_BEACON)
#endif

#define COMMON_TX_INTERVAL_SLOT						(PRIOR_TX_INTERVAL_SLOT + NUM_PRIOR_TX_INTERVAL * MIN_SIZE_DATA_BLOCK)

// ~ для SLOTS_PER_BEACON == 6; расположение CMN_SRV_DSTR(i) зависит от кол-ва фреймов в цикле
// времянка DISTR: | ~14(PRI_SRV_DSTR) | ~31(DATA_DSTR) | ~32(CMN_SRV_DSTR(0)) | (CMN_SRV_DSTR(i)) | 195(BCN_DSTR) | 200
#define PRIOR_SERVICE_DISTR_SLOT                 	(PRIOR_TX_INTERVAL_SLOT - SERVICE_DISTR_OFFSET)
#define COMMON_SERVICE_DISTR_SLOT					(COMMON_TX_INTERVAL_SLOT - SERVICE_DISTR_OFFSET)
#define DATA_DISTR_SLOT								(COMMON_SERVICE_DISTR_SLOT - DATA_DISTR_OFFSET)
#define BEACON_DISTR_SLOT                       	(SLOTS_PER_WORK_PERIOD - 5)

#define SLOTS_PER_WORK_PERIOD                       (200)

// ИВА
#define NUM_IVA           			                (2)
#define IVA_SLOT_I(slot)                            ((IVA_SLOT + ((slot) * SLOTS_PER_IVA)) % SLOTS_PER_WORK_PERIOD)

// интервалы передачи (ПИП и ОИП)
#define SLOTS_PER_ALL_COMMON_TX_INTERVAL			(SLOTS_PER_WORK_PERIOD - COMMON_TX_INTERVAL_SLOT)
#define NUM_PRIOR_TX_INTERVAL						(3) //сколько приоритетных слотов со служебкой// in MIN_SIZE_DATA_BLOCK
#define SLOTS_PER_COMMON_TX_INTERVAL(c_size)		((c_size) * MIN_SIZE_DATA_BLOCK)
#define COMMON_TX_INTERVAL_SLOT_I(i, c_size)		((COMMON_TX_INTERVAL_SLOT + (i) * SLOTS_PER_COMMON_TX_INTERVAL(c_size)) % SLOTS_PER_WORK_PERIOD)

// моменты создания задач для передачи/приема данных и служебных сообщений
#define SERVICE_DISTR_OFFSET                      	(5)
#define DATA_DISTR_OFFSET                      		(1)
#define COMMON_SERVICE_DISTR_SLOT_I(i, c_size)		((COMMON_SERVICE_DISTR_SLOT + (i) * SLOTS_PER_COMMON_TX_INTERVAL(c_size)) % SLOTS_PER_WORK_PERIOD)

// размеры событий в блоках
#define MIN_SIZE_DATA_BLOCK							(6)
#define SLOTS_SYNCHR_RESERV                         (2) // после приема пакета синхронизации нужно еще 2 инр в которых нельзя ничего делать чтобы поддернуть времянку
#define SLOTS_PER_ALL_TX_INTERVAL  					(SLOTS_PER_WORK_PERIOD - PRIOR_TX_INTERVAL_SLOT)

#define SLOTS_PER_RX_SYN_PRECONN                    (3)
#define SLOTS_PER_RX_SYN_SEARCH                     MAX_SLOTS_PER_DATA_TX//SLOTS_PER_WORK_PERIOD
#define SLOTS_PER_RX_SYN_IN_DATA_SLOTS              ( ( SLOTS_PER_ALL_TX_INTERVAL > MAX_SLOTS_PER_DATA_TX) ? MAX_SLOTS_PER_DATA_TX : SLOTS_PER_ALL_TX_INTERVAL )
#define SLOTS_PER_TX_SYN_SUPPORT                    (TX_BEACON_SLOT - TX_RESERV_SLOT) // (TX_BEACON_SLOT - (TX_SYNCHR_SLOT + 1)

#define SLOTS_PER_IVA								(3)

// todo
#define SLOTS_PER_BEACON							GetNumSlots_PRD1(PKT_HDR_LEN + PKT_BEACON_LEN, US_IN_IPS * PP_FRQ_PER_SLOT)

// старт
#define RESTART_CYCLES_RANDOMIZE_MSK                (0x04) //на сколько максимально добавлять к интервалу прослушки за счет рандома циклов * (16+1) см init_randomizer()
#define CYCLES_FOR_WAIT_SYNCHR                      ((BR_FREQ_NUM + 1) * 2) // число циклов до организации сети Start_Network()

// рестарт
#define MAX_ATOMIC_FSET_SLOTS						(MAX_SLOTS_PER_DATA_TX + MODEM_SLOT_DELAY) //для рестарта для того чтобы модем успел отработать команды до рестарта, а также успел выдать RXDATA на последнее принятое из канала
#define CYCLES_PER_SEC								(10) // длина цикла 100ms
#define RESTART_CYCLES_FOR_WAIT_CONFIRM_BCN			(3) //число групповых циклов без приема биконов во время подключения прежде чем запустить рестарт
#define RESTART_CYCLES_FOR_WAIT_FIRST_BCN      		(CYCLES_PER_SEC * 4) // никого не услышали после приема пакета синхронизации
#define RESTART_CYCLES_FOR_WAIT_FIRST_CONN_REQ		(BR_FREQ_NUM * 5) //мы создатель сети и к нам слишком долго никто не подключается
#define RESTART_NUM_OF_CONN_REQ_ATTEMPTS			(8) // количество посланных запросов до активации рестарта, до 16 фреймов

// параметры, задающие управление скоростью
#define GROUP_CYCLES_TO_RESET_SPEED                 (3) // будем выбирать минимальную скорость за данное количество групповых циклов
#define GROUP_CYCLES_TO_ANALYZE_FRQ                 (4) //(сейчас восстановление происходит за  GROUP_CYCLES_TO_ANALYZE_FRQ - 1 циклов)// будем восстанавливать частоты если с/ш у них выше порога за данное количество приемов и выкалывать если среднее за данное количество приенмев ниже порога
#define FRQ_WND_SZ            						(4) //сколько значений с/ш для частоты усреднять
#define FRQ_WND_FRBDN_IDX							(FRQ_WND_SZ) // спец. значение, означающее что по данной частоте в окно еще не было занесено значений с/ш

// потеря АС
#define BCN_GROUP_CYCL_TO_LOST_CONNECTION			(3) //сколько интервалов ждать от соседей их Beacon до момента исключения их из своей таблицы

// точность таймера
#define TIMER_NEED_NS_PRECISION                     100 //сколько нсек должно прибавляется за один тик таймера
#define PRECISION_US_TO_NS                          (1000/TIMER_NEED_NS_PRECISION)

#if defined (__OMNET__) // __OMNET__ Compiler
#define TIMER_MAX_US_PRECISION_IN_NS                100 //(сколько нсек прибавляется за один тик таймера)
#else
#define TIMER_MAX_US_PRECISION_IN_NS                1000 //(сколько нсек прибавляется за один тик таймера)
#endif

#define TIMER_MAX_CURRENT_PRECISION                 (TIMER_MAX_US_PRECISION_IN_NS/TIMER_NEED_NS_PRECISION) // сколько по TIMER_NEED_NS_PRECISION прибавляется за один тик таймера, должно быть больше или равно TIMER_NEED_NS_PRECISION

// работа с модемом
#define MODEM_SLOTS_FOR_SEND_COMMAND                (1) //За сколько ИНР заранее мы посылаем команду в модем
#define MODEM_TIME_FOR_SEND_COMMAND                 (MODEM_SLOTS_FOR_SEND_COMMAND*TIME_FOR_SLOT) //(200*PRECISION_US_TO_NS) // + us, которых должно хватать чтобы успеть зарядить задание до прерывания по высылке заданий модему

#define MODEM_SYNCHR_DELAY                          (1677*PRECISION_US_TO_NS) // через сколько после приема выдается CURRENTSTATUS
#define MODEM_SYNCHR_SLOT_DELAY                     (MODEM_SYNCHR_DELAY / TIME_FOR_SLOT) // какой инр был у передающего в момент окончания передачи синхра//в каком инр от начала приема будет CURRENTSTATUS

#define MODEM_SLOT_DELAY                            0 //через сколько ИНР после приема команды от нас модем ее исполнит( ((MODEM_TASK_DELAY - 1) / TIME_FOR_SLOT) + 1 ) // должно быть < SLOTS_ON_SUPERSLOT //количество транспортных слотов, которые можно успеть принять/передать во время кодирования/декодирования принятого

#define MODEM_TIME_PROTECT_SLOT_INTERV 				(100*PRECISION_US_TO_NS) // защитный интервал // ~300 метров на микросекунду, т.е. 30км на 100 мксек

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

// показывают какой будет цикл в момент отработки модемом задачи, если в момент зарядки задачи был 0 цикл группового цикла (цикл группового цикла меняется в Proc_PeriodElapsedInterrupt(), причем он приоритетнее отработки задач зарядки модем FSET)
#define  CYCLE_OFFSET_SYNCHR						(((TX_SYNCHR_SLOT - (MODEM_SLOT_DELAY + MODEM_SLOTS_FOR_SEND_COMMAND)) < 0) * 1)
#define  CYCLE_OFFSET_BEACON						(((TX_BEACON_SLOT - (MODEM_SLOT_DELAY + MODEM_SLOTS_FOR_SEND_COMMAND)) < 0) * 1)

// тайминги
#define SEC_PER_MIN                                 (60)
#define US_IN_IPS                                   (500)

#ifndef SLOT_RX_DELAY
#define TIME_FOR_SLOT                               (US_IN_IPS*PP_FRQ_PER_SLOT*PRECISION_US_TO_NS) //usec
//#define SLOT_RX_DELAY                               (1873*PRECISION_US_TO_NS)//1841//для 72 PKT_SLOT_LEN //3813//для 92    //3997 //для 96     //2863//для 64 байт     //4957//5007//5010    //usec          //5654 //usec with Calibration
//#define TIME_PROTECT_SLOT_INTERV                    ( (((TIME_FOR_SLOT-SLOT_RX_DELAY)>>1) / TIMER_MAX_CURRENT_PRECISION) * TIMER_MAX_CURRENT_PRECISION )
#endif

#define TIME_FOR_IPS                                (US_IN_IPS * PRECISION_US_TO_NS) // в сотнях наносек
#define IPS_RX_DELAY                                (TIME_FOR_IPS * 94 / 100) // 94% of IPS
#define TIME_PROTECT_IPS_INTERV                     ((TIME_FOR_IPS-IPS_RX_DELAY)>>1)
#define TIME_PROTECT_FRAME_INTERV                   0//5000//100     // добавляется в конце каждого фрейма
#define TIME_WORK_PERIOD                            (TIME_FOR_SLOT*SLOTS_PER_WORK_PERIOD)//(TIME_FRAME*MAX_FRAMES)  //     (TIME_TO_RXTX_DATA+(TIME_FRAME*(MAX_FRAMES-1))) // Frame №0 for Beacons RX/TX
#define TIME_TO_SYNC                                ((TIME_WORK_PERIOD)-(500*PRECISION_US_TO_NS))         //((TIME_FRAME*MAX_FRAMES)-TIME_PROTECT_FRAME_INTERV)  //TIME_FOR_RXTX_BEACONS

// ППРЧ
#define SLOTS_TO_WAIT_FHSS_INIT                     (2) //2 INR = 4 msec
#define FHSS_GEN_STEPS_PER_SLOT                     (500) // сколько шагов Вадим успевает сгенерировать на втором проце за ИНР

// задачник
#define SLOT_TO_START_FIRST_TASK                    (SLOTS_PER_WORK_PERIOD - MODEM_SLOT_DELAY - MODEM_SLOTS_FOR_SEND_COMMAND)
#define SLOT_TO_START_MAIN_TIMER                    (SLOT_TO_START_FIRST_TASK - 1 - SLOTS_TO_WAIT_FHSS_INIT)
//#define TIME_TO_START_FIRST_TASK                  (SLOT_TO_START_FIRST_TASK * TIME_FOR_SLOT)
//#define TIME_TO_START_MAIN_TIMER                  (SLOT_TO_START_MAIN_TIMER * TIME_FOR_SLOT)

//#if (BEACON_TX_SPEED == UCOS_KSS_RATE_PRD1)
//
//#define SLOTS_PER_BEACON							GetNumSlots_PRD1(PKT_HDR_LEN + PKT_BEACON_LEN, US_IN_IPS * PP_FRQ_PER_SLOT)
//
//#elif (BEACON_TX_SPEED == UCOS_KSS_RATE_PRD2)
//
//#define SLOTS_PER_BEACON							GetNumSlots_PRD2(PKT_HDR_LEN + PKT_BEACON_LEN, US_IN_IPS * PP_FRQ_PER_SLOT)
//
//#elif (BEACON_TX_SPEED == UCOS_KSS_RATE_PRD3)
//
//#define SLOTS_PER_BEACON							GetNumSlots_PRD3(PKT_HDR_LEN + PKT_BEACON_LEN, US_IN_IPS * PP_FRQ_PER_SLOT)
//
//#else
//	#error
//#endif


#if !defined (__OMNET__)        // OMNET Compiler
//!!!CONVERTER_START_FIELD_->

uint16_t Recalc_Time_forModemDelay(uint16_t slot);

//!!!CONVERTER_END_FIELD_<-
#endif

#endif // #ifndef __MAC_TIMELINE_H
