#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QObject>
#include <stdint.h>// uint16_t, uint8_t
#include <QVector>
#include <QFile>
#include <QTimer>

#define VSV_BITS                    4
#define VSV_MSK                     ((1 << VSV_BITS) - 1)
#define PKT_TYPE_TYPE               PktType

class FileParser : public QObject
{
    Q_OBJECT

private:
    static const int MAX_RADIO_CONNECTIONS {7};
    static const int FRQ_ARR_SZ            {8};
    static const int lengthSynhr           {68};
    static const int IPv4                  {0x0008};
    static const int SOFT_ADDR_BRDCST      {0xFFFF};

    static const int updatePBarMsec        {500};

    static const int startOffset           {24};
    static const int framesOffset          {16};
    static const int lengthFramOffs        {8};
    static const int typeFrameOffs         {12};
    static const int UCOSOffset            {42};
    static const int PSPDOffset            {54};
    static const int sizeByteBits          {8};
    static const int sizeIntBytes          {4};
    static const int sizeShortBytes        {2};

public:
    typedef  uint16_t PKT_ADDR_TYPE;
    typedef  uint16_t PKT_LEN_TYPE;
    typedef  uint8_t  PKT_LENTONEXT_TYPE;
    typedef  uint16_t PKT_CRC_TYPE;

#pragma pack(push, 1)
    // Команды управления УЦОС
    enum Modem_CMD_Names
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
    };

    enum PktType :uint8_t
    {
        PKT_TYPE_BEACON				= 1,	// Brdcst
        PKT_TYPE_RTS				= 2,	// Uncst
        PKT_TYPE_CTS				= 3,	// Uncst/Brdcst
        PKT_TYPE_ACK				= 4,	// Brdcst
        PKT_TYPE_NACK				= 5,	// Brdcst
        PKT_TYPE_VIRT_CH_READY		= 6,	// Uncst
        PKT_TYPE_DATA				= 7,	// Uncst/Mltcst/Brdcst
        PKT_TYPE_DATA_TIMEMARKED	= 8,	// Uncst/Mltcst/Brdcst
        PKT_TYPE_REQUEST			= 9,	// Uncst/Mltcst/Brdcst
        PKT_TYPE_DATA_MAP			= 10,	// Brdcst
        PKT_TYPE_DATA_MAP_REQUEST	= 11,	// Brdcst
        PKT_TYPE_CONNECT_REQUEST	= 12,	// Brdcst
        PKT_TYPE_TIME_CORRECTION	= 13,	// Brdcst
        PKT_TYPE_RREQ				= 14,	// запрос на маршрут Brdcst
        PKT_TYPE_RREP				= 15,	// ответ на запрос Uncst
        PKT_TYPE_RERR				= 16,	// ошибка маршрута Uncst/Brdcst
        PKT_TYPE_MAC_IP_REQ			= 17,	// запрос mac ip
        PKT_TYPE_MAC_IP_REP			= 18,	// ответ mac ip
        PKT_TYPE_NET_MAP			= 19,	// список сетевых адресов
    };

    enum VSV_t :uint8_t
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
    } ;

    enum TraffDirectionType :uint8_t
    {
        DIRECTION_TYPE_RESERV	= 0x00,
        DIRECTION_TYPE_RX		= 0x01,
        DIRECTION_TYPE_TX		= 0x02,
        DIRECTION_TYPE_DUPLEX	= 0x03,
    };

    enum Reserv_elem_type_t
    {
        RE_CURRENT				,	// текущая ЗоР
        RE_PLANNED 				,	// планируемая ЗоР
        RE_CLOSING 				,	// вспомогательная ЗоР, для закрытия временного участка
        RE_RESERVED
    };

    enum ReservingResult_t
    {
        RESULT_DISSENT 		= 0,	// не согласны
        RESULT_CONSENT 		= 1,	// согласны
        RESULT_2			= 2
    };

    enum ClosingReason_t :uint8_t
    {
        REASON_TRAFF_END 		= 0,
        REASON_ERROR_RESERVING	= 1,
        REASON_REPLACE			= 2,
        REASON_NOT_EXISTS		= 3,
        REASON_TRAFF_END_REP	= 4,
    };

    struct Pkt_Hdr
    {
        PKT_ADDR_TYPE DestAddr;
        PKT_ADDR_TYPE SrcAddr;
        PKT_LEN_TYPE PktLen;
        PKT_CRC_TYPE crc;
        PKT_TYPE_TYPE Data_type;
        PKT_LENTONEXT_TYPE Data_LenToNext;
    };

    struct Pkt_type_Hdr
    {
        PKT_TYPE_TYPE Type;
        PKT_LENTONEXT_TYPE LenToNext;
        uint8_t pdata[];
    };

    struct Network_Par_t
    {
        uint8_t					max_as;				// необязательное поле, для wireshark
        uint8_t					curr_frame;
        uint8_t					curr_cycle_size;
        uint8_t					next_cycle_size;
        uint8_t					next_bcn_cycle;		// неясно, так ли необходимо это поле, только если бикон должен быть послан в следующем цикле и во фрейме, значение которого превышает значение фрейма при приеме
        uint8_t					next_bcn_frame;
    };

    struct AS_State_t
    {
        uint16_t addr :16;
    };

    struct AS_Speed_t
    {
        uint8_t possible_rx_rate :3;
        uint8_t next_cycle_tx_rate :3;
    };

    struct Pkt_Beacon
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
    };

    struct Reservation_entry_t
    {
        uint8_t 				num_tr_blocks;	// кол-во транспортных блоков // если кол-во ТБ равно 0, значит ЗоР пустая
        uint8_t 				offset;			// смещение

        uint8_t 				freq_ch			:4; // частотный канал
        TraffDirectionType		direction		:2; // направление (00-Резерв, 01-Прм, 02-Прд, 03-дуплекс )
        Reserv_elem_type_t		type			:2;	// тип ЗоР: текущая, планируемая, вспомогательная (+) или (-)

        uint8_t 				rate			:2; // скорость (0,1,2...)
        uint8_t					reserved		:6;
    };

    struct Pkt_Service
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
    };

#pragma pack(pop)

    struct mapAddrPkt_t
    {
        uint16_t                addrFrom;
        uint8_t                 PktNumber;
        int                     numbFrameBeac;
    };

    struct missedPkt_t
    {
        uint16_t                addr;
        QVector<mapAddrPkt_t>   PktNumbers;
    };


    struct counter_t
    {
        uint16_t                addr;
        uint8_t                 CurrentPktNumber;
    };

public:
    explicit FileParser(QObject *parent = nullptr);
    ~FileParser();

    counter_t *getCounter(uint16_t addr);
    void setCounter(uint16_t addr, uint8_t PktNumber);

    missedPkt_t *getMissedPkt(uint16_t addr);
    void setMissedPkt(uint16_t addr, uint16_t addrFrom, uint8_t PktNumber, int numbFrameBeac);

    QVector<missedPkt_t> &getMissedPkts(){return m_missedPkts;}

    void readFile(QFile &file);

signals:
    void timeout(uint64_t readBytes);

private:
    QTimer                  *m_timer;

    QVector<counter_t>      m_counters;
    QVector<missedPkt_t>    m_missedPkts;
    uint64_t                m_readBytes;

    void parseFrame(const QByteArray &frame, int numbFrame);
    void parseService(Pkt_Hdr *PktData);
    void parseBeacon(Pkt_Hdr *PktData, int numbFrame);
    int bytesToInt(const QByteArray &bytes, int first, int sizeInt);
    bool CompNum8(uint8_t Num1, uint8_t Num2);
    VSV_t get_VSV(uint8_t *p_vsv, uint8_t index);
};

#endif // FILEPARSER_H
