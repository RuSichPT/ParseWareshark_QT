
//MAC_Timeline.h//

//Define to prevent recursive inclusion
#ifndef __MAC_TIMELINE_H
#define __MAC_TIMELINE_H

#include "external_files/MAC/Inc/MAC_debug.h"
#include "external_files/MAC/Inc/MAC_Max_STA_Num.h"	//MAX_RADIO_CONNECTIONS
#include "external_files/MAC/Inc/ModemProtocol.h"		//PP_FRQ_PER_SLOT, GetSlot_PRD1(), ..
#include "external_files/MAC/Inc/MAC_pkts_format.h"	//PKT_BEACON_LEN


// входные данные:

// времянка:
#define SLOTS_PER_WORK_PERIOD                       (200)

#define US_IN_IPS                                   (500)

#if defined(MODEM_SBS)
#define TX_RESERV_SLOTS								(2) // сколько нужно ИПС, в которых нельзя ничего делать, после приема пакета синхронизации, чтобы поддернуть времянку

#elif defined(MODEM_PANZYR_SM)
#define TX_RESERV_SLOTS								(2) // сколько нужно ИПС, в которых нельзя ничего делать, после приема пакета синхронизации, чтобы поддернуть времянку

#elif defined(MODEM_CC1312)
//для сс1312 нужно чтобы в следущем после приёма синхра ИПСе не было команд.
//т.к. команда идет по UART1, генерит прерывание, у которого приоритет выше калбэка на выдачу данных по синхру, и прерывание затягивает выдачу данных по синхру и сбивает времянку
//теоретически можно командой уехать влево от ИПСа, идущего после приёма синхра
#define TX_RESERV_SLOTS								(MODEM_SLOTS_FOR_SEND_COMMAND+1) // сколько нужно ИПС, в которых нельзя ничего делать, после приема пакета синхронизации, чтобы поддернуть времянку

#else
#error
#endif// defined(MODEM_CC1312)

#define MIN_SIZE_DATA_BYTES							(PKT_DATA_WITH_HDR_SZ + 42)// 57, минимальный размер передаваемого без резервирования пакета (ARP)

#define NUM_IVA           			                (2) // сколько нужно интервалов ввода абонентов
#define NUM_PRIOR_TX_INTERVAL						(3) // сколько приоритетных слотов со служебкой// in MIN_SIZE_DATA_BLOCK

#define SERVICE_DISTR_OFFSET                      	(5) // за сколько ИПС до PRIOR_TX_INTERVAL нужно обработать задачи на генерацию служебки
#define DATA_DISTR_OFFSET                      		(1) // Формирование задач на приём/передачу ТРАФИКА, должно происходить до COMMON_SERVICE_DISTR_START(0), чтобы если на ближайшее место уже разместили данные, для служебки было бы занято, чтобы не было одновременной передачи
#define BEACON_DISTR_SLOT                       	(SLOTS_PER_WORK_PERIOD - 5)
//#define ORDER_BEACON_IVA                       		// взаимное расположение BEACON и IVA на ВД: BEACON -> IVA или IVA -> BEACON(по-умолчанию)

// работа с модемом
#define MODEM_SLOTS_FOR_SEND_COMMAND                (1) //За сколько ИНР заранее мы посылаем команду в модем
#define MODEM_TIME_PROTECT_SLOT_INTERV 				(100*PRECISION_US_TO_NS) // защитный интервал // ~300 метров на микросекунду, т.е. 30км на 100 мксек
#define IPS_RX_DELAY                                (TIME_FOR_IPS * 94 / 100) // 94% of IPS
// ППРЧ
#define SLOTS_TO_WAIT_FHSS_INIT                     (2) //2 INR = 4 msec
#define FHSS_GEN_STEPS_PER_SLOT                     (500) // сколько шагов Вадим успевает сгенерировать на втором проце за ИНР

// старт
#define RESTART_CYCLES_RANDOMIZE_MSK                (0x04) //сколько циклов по (BR_FREQ_NUM + 1) максимально добавлять к интервалу прослушки за счет рандома см init_randomizer()
#define CYCLES_FOR_WAIT_SYNCHR                      ((BR_FREQ_NUM + 1) * 2) // минимальное число циклов до организации сети Start_Network()

// рестарт
#define RESTART_CYCLES_FOR_WAIT_CONFIRM_BCN			(3) //число групповых циклов без приема биконов во время подключения прежде чем запустить рестарт
#define RESTART_CYCLES_FOR_WAIT_FIRST_BCN      		(CYCLES_PER_SEC * 4) // никого не услышали после приема пакета синхронизации
#define RESTART_CYCLES_FOR_WAIT_FIRST_CONN_REQ		(BR_FREQ_NUM * 5) //мы создатель сети и к нам слишком долго никто не подключается
#define RESTART_NUM_OF_CONN_REQ_ATTEMPTS			(8) // количество посланных запросов до активации рестарта, до 16 фреймов

// параметры, задающие управление скоростью
#define GROUP_CYCLES_TO_RESET_SPEED                 (3) // будем выбирать минимальную скорость за данное количество групповых циклов
#define GROUP_CYCLES_TO_ANALYZE_FRQ                 (4) //(сейчас восстановление происходит за  GROUP_CYCLES_TO_ANALYZE_FRQ - 1 циклов)// будем восстанавливать частоты если с/ш у них выше порога за данное количество приемов и выкалывать если среднее за данное количество приемов ниже порога
#define FRQ_WND_SZ            						(4) //сколько значений с/ш для частоты усреднять

// потеря АС
#define BCN_GROUP_CYCL_TO_LOST_CONNECTION			(3) //сколько интервалов ждать от соседей их Beacon до момента исключения их из своей таблицы

// точность таймера
#if defined (__OMNET__) // __OMNET__ Compiler
#define TIMER_MAX_US_PRECISION_IN_NS                100 //(сколько нсек прибавляется за один тик таймера)
#else
#define TIMER_MAX_US_PRECISION_IN_NS                1000 //(сколько нсек прибавляется за один тик таймера)
#endif

#define TIMER_NEED_NS_PRECISION                     100 //сколько нсек должно прибавляется за один тик таймера
#define PRECISION_US_TO_NS                          (1000/TIMER_NEED_NS_PRECISION)

///////////////////////////////////////////////////////////////////////

// вычисляемые данные:

// ~ для SLOTS_PER_BEACON == 6, расположение CMN_INT(i) зависит от кол-ва фреймов в цикле
// времянка TX/RX: | 0(SYNC) | 4(RSRV) | 7(BCN) | ~13(IVA) | ~19(PRI_INT) | ~37(CMN_INT(0)) | (CMN_INT(i)) | 200

//SYNCHR
#define TX_SYNCHR_SLOT                       		(0)
#define SLOTS_PER_SYNCHR							GET_SLOTS_MIN_SPEED_SYNC(PKT_SYNCHR_LEN, US_IN_IPS * PP_FRQ_PER_SLOT)//3
#define SLOTS_PER_RX_SYN_PRECONN                    (SLOTS_PER_SYNCHR)//3
#define SLOTS_PER_RX_SYN_SEARCH                     MAX_SLOTS_PER_DATA_TX//SLOTS_PER_WORK_PERIOD
#define SLOTS_PER_RX_SYN_IN_DATA_SLOTS              ( ( SLOTS_PER_ALL_TX_INTERVAL > MAX_SLOTS_PER_DATA_TX) ? MAX_SLOTS_PER_DATA_TX : SLOTS_PER_ALL_TX_INTERVAL )
#define SLOTS_PER_TX_SYN_SUPPORT                    (TX_BEACON_SLOT - TX_RESERV_SLOT) // (TX_BEACON_SLOT - (TX_SYNCHR_SLOT + 1)

//RESERV
#define TX_RESERV_SLOT                       		(TX_SYNCHR_SLOT + SLOTS_PER_SYNCHR)//4


//BEACON / IVA / PRIOR_TX_INTERVAL
#define SLOTS_PER_BEACON							GET_SLOTS_MIN_SPEED(PKT_HDR_LEN + PKT_BEACON_LEN, US_IN_IPS * PP_FRQ_PER_SLOT)//6
#define SLOTS_PER_IVA								GET_SLOTS_MIN_SPEED(PKT_HDR_LEN + PKT_CON_REQ_LEN, US_IN_IPS * PP_FRQ_PER_SLOT)//3

#ifdef ORDER_BEACON_IVA
#define TX_BEACON_SLOT                       		(TX_RESERV_SLOT + TX_RESERV_SLOTS)
#define IVA_SLOT                    				(TX_BEACON_SLOT + SLOTS_PER_BEACON)
#define PRIOR_TX_INTERVAL_SLOT						IVA_SLOT_I(NUM_IVA)
#else
// времянка TX/RX: | 0(SYNC) | 4(RSRV) | 7(IVA) | 13(BCN) | ~19(PRI_INT) | ~37(CMN_INT(0)) | (CMN_INT(i)) | 200
#define IVA_SLOT                    				(TX_RESERV_SLOT + TX_RESERV_SLOTS)
#define TX_BEACON_SLOT                       		IVA_SLOT_I(NUM_IVA)
#define PRIOR_TX_INTERVAL_SLOT						(TX_BEACON_SLOT + SLOTS_PER_BEACON)
#endif//#ifdef ORDER_BEACON_IVA
// ИВА
#define IVA_SLOT_I(slot)                            ((IVA_SLOT + ((slot) * SLOTS_PER_IVA)) % SLOTS_PER_WORK_PERIOD)

//COMMON_TX_INTERVAL
#define COMMON_TX_INTERVAL_SLOT						(PRIOR_TX_INTERVAL_SLOT + (NUM_PRIOR_TX_INTERVAL * MIN_SIZE_DATA_BLOCK))
#define SLOTS_PER_COMMON_TX_INTERVAL(c_size)		((c_size) * MIN_SIZE_DATA_BLOCK)
#define COMMON_TX_INTERVAL_SLOT_I(i, c_size)		((COMMON_TX_INTERVAL_SLOT + (i) * SLOTS_PER_COMMON_TX_INTERVAL(c_size)) % SLOTS_PER_WORK_PERIOD)

// ~ для SLOTS_PER_BEACON == 6; расположение CMN_SRV_DSTR(i) зависит от кол-ва фреймов в цикле
// времянка DISTR: | ~14(PRI_SRV_DSTR) | ~31(DATA_DSTR) | ~32(CMN_SRV_DSTR(0)) | (CMN_SRV_DSTR(i)) | 195(BCN_DSTR) | 200
// моменты создания задач для передачи/приема данных и служебных сообщений
#define PRIOR_SERVICE_DISTR_SLOT                 	(PRIOR_TX_INTERVAL_SLOT - SERVICE_DISTR_OFFSET)
#define COMMON_SERVICE_DISTR_SLOT					(COMMON_TX_INTERVAL_SLOT - SERVICE_DISTR_OFFSET)
#define DATA_DISTR_SLOT								(COMMON_SERVICE_DISTR_SLOT - DATA_DISTR_OFFSET)

#define COMMON_SERVICE_DISTR_SLOT_I(i, c_size)		((COMMON_SERVICE_DISTR_SLOT + (i) * SLOTS_PER_COMMON_TX_INTERVAL(c_size)) % SLOTS_PER_WORK_PERIOD)

// размеры событий в блоках
#define MIN_SIZE_DATA_BLOCK							GET_SLOTS_MIN_SPEED(MIN_SIZE_DATA_BYTES, US_IN_IPS * PP_FRQ_PER_SLOT)//(6)
#define SLOTS_PER_ALL_TX_INTERVAL  					(SLOTS_PER_WORK_PERIOD - PRIOR_TX_INTERVAL_SLOT)

// интервалы передачи (ПИП и ОИП)
#define SLOTS_PER_ALL_COMMON_TX_INTERVAL			(SLOTS_PER_WORK_PERIOD - COMMON_TX_INTERVAL_SLOT)

// рестарт
#define MAX_ATOMIC_FSET_SLOTS						(MAX_SLOTS_PER_DATA_TX + MODEM_SLOT_DELAY) //для рестарта для того чтобы модем успел отработать команды до рестарта, а также успел выдать RXDATA на последнее принятое из канала
#define CYCLES_PER_SEC								(1000000/(SLOTS_PER_WORK_PERIOD*US_IN_IPS))//(10) // длина цикла 100ms

// параметры, задающие управление скоростью
#define FRQ_WND_FRBDN_IDX							(FRQ_WND_SZ) // спец. значение, означающее что по данной частоте в окно еще не было занесено значений с/ш

// точность таймера
#define TIMER_MAX_CURRENT_PRECISION                 (TIMER_MAX_US_PRECISION_IN_NS/TIMER_NEED_NS_PRECISION) // сколько по TIMER_NEED_NS_PRECISION прибавляется за один тик таймера, должно быть больше или равно TIMER_NEED_NS_PRECISION

// работа с модемом
#define MODEM_TIME_FOR_SEND_COMMAND                 (MODEM_SLOTS_FOR_SEND_COMMAND*TIME_FOR_SLOT) //(200*PRECISION_US_TO_NS) // + us, которых должно хватать чтобы успеть зарядить задание до прерывания по высылке заданий модему
#define MODEM_SYNCHR_SLOT_DELAY                     (MODEM_SYNCHR_DELAY / TIME_FOR_SLOT) // какой инр был у передающего в момент окончания передачи синхра//в каком инр от начала приема будет CURRENTSTATUS

// показывают какой будет цикл в момент отработки модемом задачи, если в момент зарядки задачи был 0 цикл группового цикла (цикл группового цикла меняется в Proc_PeriodElapsedInterrupt(), причем он приоритетнее отработки задач зарядки модем FSET)
#define  CYCLE_OFFSET_SYNCHR						(((TX_SYNCHR_SLOT - (MODEM_SLOT_DELAY + MODEM_SLOTS_FOR_SEND_COMMAND)) < 0) * 1)
#define  CYCLE_OFFSET_BEACON						(((TX_BEACON_SLOT - (MODEM_SLOT_DELAY + MODEM_SLOTS_FOR_SEND_COMMAND)) < 0) * 1)

// тайминги
#define SEC_PER_MIN                                 (60)

#ifndef SLOT_RX_DELAY
#define TIME_FOR_SLOT                               (US_IN_IPS*PP_FRQ_PER_SLOT*PRECISION_US_TO_NS) //в сотнях наносек
//#define SLOT_RX_DELAY                               (1873*PRECISION_US_TO_NS)//1841//для 72 PKT_SLOT_LEN //3813//для 92    //3997 //для 96     //2863//для 64 байт     //4957//5007//5010    //usec          //5654 //usec with Calibration
//#define TIME_PROTECT_SLOT_INTERV                    ( (((TIME_FOR_SLOT-SLOT_RX_DELAY)>>1) / TIMER_MAX_CURRENT_PRECISION) * TIMER_MAX_CURRENT_PRECISION )
#endif

#define TIME_FOR_IPS                                (US_IN_IPS * PRECISION_US_TO_NS) // в сотнях наносек
#define TIME_PROTECT_IPS_INTERV                     ((TIME_FOR_IPS-IPS_RX_DELAY)>>1)
#define TIME_PROTECT_FRAME_INTERV                   0//5000//100     // добавляется в конце каждого фрейма
#define TIME_WORK_PERIOD                            (TIME_FOR_SLOT*SLOTS_PER_WORK_PERIOD)//(TIME_FRAME*MAX_FRAMES)  //     (TIME_TO_RXTX_DATA+(TIME_FRAME*(MAX_FRAMES-1))) // Frame №0 for Beacons RX/TX
#define TIME_TO_SYNC                                ((TIME_WORK_PERIOD)-(TIME_FOR_IPS))         //((TIME_FRAME*MAX_FRAMES)-TIME_PROTECT_FRAME_INTERV)  //TIME_FOR_RXTX_BEACONS


// задачник
#define SLOT_TO_START_FIRST_TASK                    (SLOTS_PER_WORK_PERIOD - MODEM_SLOT_DELAY - MODEM_SLOTS_FOR_SEND_COMMAND)
#define SLOT_TO_START_MAIN_TIMER                    (SLOT_TO_START_FIRST_TASK - 1 - SLOTS_TO_WAIT_FHSS_INIT)
//#define TIME_TO_START_FIRST_TASK                  (SLOT_TO_START_FIRST_TASK * TIME_FOR_SLOT)
//#define TIME_TO_START_MAIN_TIMER                  (SLOT_TO_START_MAIN_TIMER * TIME_FOR_SLOT)

//#if (BEACON_TX_SPEED == UCOS_KSS_RATE_PRD1)
//
//#define SLOTS_PER_BEACON							GET_SLOTS_MIN_SPEED(PKT_HDR_LEN + PKT_BEACON_LEN, US_IN_IPS * PP_FRQ_PER_SLOT)
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

// для FSET команд
#define ID_CYCLE_SHFT								9		//на сколько бит двигать номер цикла, или сколько бит из 16 на отображение номера слота в ID команд
#define ID_SLOT_MSK									((1 << ID_CYCLE_SHFT) - 1)
#define ID_CYCLE_MSK								((1 << (16 - ID_CYCLE_SHFT)) - 1)

#define ID_UNIQUE_CMD								SLOTS_PER_WORK_PERIOD//уникальный ИД, число, которое не может быть сгенерировано generate_cmd_ID()

#define BEACON_TB_NUM								(0)
#define DATA_TB_NUM									(0)


#if !defined (__OMNET__)        // OMNET Compiler
//!!!CONVERTER_START_FIELD_->

uint16_t Recalc_Time_forModemDelay(uint16_t slot);

//!!!CONVERTER_END_FIELD_<-
#endif

#endif // #ifndef __MAC_TIMELINE_H
