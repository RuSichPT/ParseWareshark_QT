#include "inc/fileparser.h"
#include "inc/mywidget.h"
#include "external_files/MAC/Inc/ModemProtocol.h" //UCOS_CMD_TXDATA
#include <QDebug>

typedef FileParser::Counter_t       Counter_t;
typedef FileParser::MissedPkt_t     MissedPkt_t;
typedef FileParser::RepeatedPkt_t   RepeatedPkt_t;

FileParser::FileParser(QObject *parent)
    : QObject(parent)
    , m_readBytes(0)
{
    // Таймер обновления прогресс бара
    m_timer = new QTimer(this);
    connect(this, &FileParser::timeout, static_cast<MyWidget*>(parent), &MyWidget::updateBar,Qt::DirectConnection);
}

FileParser::~FileParser()
{

}

Counter_t* FileParser::getCounter(uint16_t addr)
{
    if(!m_counters.isEmpty())
    {
        for (int var = 0; var < m_counters.size(); ++var)
        {
            if(m_counters.at(var).addr == addr)
            {
                return &m_counters[var];
            }
        }
    }

    return nullptr;
}

void FileParser::setCounter(uint16_t addr, uint8_t pktNumber, int frameNumber)
{
    Counter_t *counter = getCounter(addr);

    if (counter == nullptr)
    {
        counter = createCounter(addr, pktNumber);
    }
    else
    {
        updateCurrentNumber(counter, pktNumber);
    }

    counter->matchPktToFrame[pktNumber] = frameNumber;
}

Counter_t* FileParser::createCounter(uint16_t addr, uint8_t pktNumber)
{
    Counter_t counter;
    counter.addr = addr;
    counter.currentPktNumber = pktNumber;

    for(int i = 0; i < SIZE_MATCH; i++)
    {
        counter.matchPktToFrame[i] = 0;
    }

    m_counters.append(counter);

    return &m_counters[(m_counters.size()) - 1];
}

void FileParser::updateCurrentNumber(Counter_t *counter, uint8_t pktNumber)
{
    if (CompNum8(pktNumber, counter->currentPktNumber))
    {
        counter->currentPktNumber = pktNumber;

        if (pktNumber == 0)
        {
            counter->currentCycle++;
        }
    }
}

MissedPkt_t* FileParser::getMissedPkt(uint16_t srcAddr, uint8_t pktNumber, int cycle)
{
    if (!m_missedPkts.isEmpty())
    {
        for (int i = 0; i < m_missedPkts.size(); i++)
        {
            if (m_missedPkts.at(i).srcAddr == srcAddr
                    && m_missedPkts.at(i).pktNumber == pktNumber
                    && m_missedPkts.at(i).cycle == cycle)
            {
                return &m_missedPkts[i];
            }
        }
    }

    return nullptr;
}

void FileParser::setMissedPkt(uint16_t destAddr, uint16_t srcAddr, uint8_t pktNumber, int beacFrameNum, int pktFrameNum, int cycle)
{
    MissedPkt_t missedPkt;

    missedPkt.destAddr = destAddr;
    missedPkt.srcAddr = srcAddr;
    missedPkt.pktNumber = pktNumber;
    missedPkt.pktFrameNumber = pktFrameNum;
    missedPkt.beacFrameNumber = beacFrameNum;
    missedPkt.cycle = cycle;

    m_missedPkts.append(missedPkt);
}

void FileParser::setRepeatedPkt(uint16_t srcAddr, uint8_t pktNumber, int pktFrameNum)
{
    RepeatedPkt_t repeatedPkt;

    repeatedPkt.srcAddr = srcAddr;
    repeatedPkt.pktNumber = pktNumber;
    repeatedPkt.pktFrameNumber = pktFrameNum;

    m_repeatedPkts.append(repeatedPkt);
}

void FileParser::readFile(QFile &file)
{
    if (file.exists())
    {
        QByteArray frameOf, frame;
        int lengthFrame;
        int numFrame {1};

        // Запускаем таймер
        m_timer->start(UPDATE_PBAR_MSEC);
        file.read(START_OFFSET);
        m_readBytes += START_OFFSET;
        while(!file.atEnd())
        {
            if (m_timer->remainingTime() != 0)
            {
                frameOf = file.read(FRAMES_OFFSET);
                // Узнаем длину фрейма
                lengthFrame = bytesToInt(frameOf, LEN_FRAME_OFFSET, SIZE_INT_BYTES);
                frame = file.read(lengthFrame);
                parseFrame(frame, numFrame);
                numFrame++;
                m_readBytes += FRAMES_OFFSET + lengthFrame;
            }
            else
            {
                emit timeout(m_readBytes);
                m_timer->start(UPDATE_PBAR_MSEC);
            }
        }

        emit timeout(m_readBytes);
        m_timer->stop();
    }
}

void FileParser::parseFrame(const QByteArray &frame, int numFrame)
{
    // Узнаем тип
    int typeFrame = bytesToInt(frame, TYPE_FRAME_OFFSET, SIZE_SHORT_BYTES);
    if (typeFrame == PP_HTONS(ETHTYPE_IP))
    {
        // Узнаем Modem_CMD_Name
        int modemCMDName = bytesToInt(frame, UCOS_OFFSET, SIZE_SHORT_BYTES);
        if (modemCMDName == UCOS_CMD_TXDATA)
        {
            // Исключаем пакет синхронизации
            if (frame.size() != LEN_SYNHR)
            {
                const char *pFrame;
                Pkt_Hdr *PktData;

                pFrame = frame.data();
                pFrame += PSPD_OFFSET;
                PktData = (Pkt_Hdr *)pFrame;
                parseService(PktData, numFrame);
                parseBeacon(PktData, numFrame);
            }
        }
    }
}

void FileParser::parseService(Pkt_Hdr *PktData, int numFrame)
{
    Pkt_type_Hdr* type_Hdr;
    uint16_t LenToNext_field{0};

    type_Hdr = (Pkt_type_Hdr*)&(PktData->Data_type);

    if (PktData->DestAddr == SOFT_ADDR_BRDCST)
    {
        do //Piggy-back по PKTdata->pData_LenToNext - по разным типам пакетов внутри одного пакета с хедером
        {
            LenToNext_field = type_Hdr->LenToNext;

            // Смотрим все служебные пакеты и запоминаем номер последнего пакета от абонента
            if ( (type_Hdr->Type == PKT_TYPE_CTS)
                 || (type_Hdr->Type == PKT_TYPE_RTS)
                 || (type_Hdr->Type == PKT_TYPE_ACK)
                 || (type_Hdr->Type == PKT_TYPE_NACK)
                 || (type_Hdr->Type == PKT_TYPE_NACK_R)
                 || (type_Hdr->Type == PKT_TYPE_DATA_MAP)
                 || (type_Hdr->Type == PKT_TYPE_DATA_MAP_REQUEST)
                 || (type_Hdr->Type == PKT_TYPE_CONTROL_NUMBER)
                 || (type_Hdr->Type == PKT_TYPE_FORCE_RTS)
                 || (type_Hdr->Type == PKT_TYPE_FORCE_CTS)
                 )
            {
                uint8_t pktNumber;

                if (type_Hdr->Type == PKT_TYPE_CONTROL_NUMBER)
                {
                    Pkt_control_number *controlPkt = (Pkt_control_number *)(type_Hdr);
                    pktNumber = controlPkt->pkt_number;
                }
                else
                {
                    Pkt_Service *servicePkt = (Pkt_Service *)(type_Hdr);
                    pktNumber = servicePkt->pkt_number;
                }

                setCounter(PktData->SrcAddr, pktNumber, numFrame);
                Counter_t *counter = getCounter(PktData->SrcAddr);

                if (getMissedPkt(PktData->SrcAddr, pktNumber, counter->currentCycle) != nullptr)
                {

                    setRepeatedPkt(PktData->SrcAddr, pktNumber, numFrame);
                }
            }
            type_Hdr = (Pkt_type_Hdr*) ( ((uint8_t*)(&(type_Hdr->LenToNext))) + sizeof(PKT_LENTONEXT_TYPE) + LenToNext_field );
        }
        while (LenToNext_field != 0);
    }
}

void FileParser::parseBeacon(Pkt_Hdr *PktData, int beacFrameNum)
{
    if (PktData->DestAddr == SOFT_ADDR_BRDCST)
    {
        // Смотрим пропуски пакетов
        if ( PktData->Data_type == PKT_TYPE_BEACON)
        {
            Pkt_Beacon *Beacon = (Pkt_Beacon *)&(PktData->Data_type);

            for (int var = 0; var < MAX_RADIO_CONNECTIONS; var++)
            {
                Counter_t *counter = getCounter(Beacon->AS[var].addr);
                if (counter != nullptr)
                {
                    // Не смотрим пропуски у источника бикона
                    if(PktData->SrcAddr != counter->addr)
                    {
                        // Проверяем вид связи на абонента
                        if (    (get_VSV(Beacon->VSV, var) == VSV_RSV)
                                || (get_VSV(Beacon->VSV, var) == VSV_RPSV)
                            )
                        {
                            if (Beacon->last_heard_pkt_number[var] != counter->currentPktNumber)
                            {
                                int pktNumber = Beacon->last_heard_pkt_number[var] + 1;
                                int pktFrameNum = counter->matchPktToFrame[pktNumber];
                                setMissedPkt(PktData->SrcAddr, counter->addr, pktNumber, beacFrameNum, pktFrameNum, counter->currentCycle);
                            }
                        }
                    }
                }
            }
        }
    }
}

int FileParser::bytesToInt(const QByteArray &bytes, int first, int sizeInt)
{
    // Конвертирует size байта QByteArray в число int
    int result {0};
    int tmp {0};
    for (int i = 0; i < sizeInt; i++)
    {
        tmp = static_cast<uint8_t>(bytes.at(first+i));
        result = result | (tmp << i*SIZE_BYTE_BITS);
    }
    return result;
}

bool FileParser::CompNum8(uint8_t Num1, uint8_t Num2)
{
    // Если true то SeqNum1 > SeqNum2, Если false то SeqNum2 > SeqNum1

    if ( (int8_t)(Num1 - Num2) > 0 )
        return true;
    else
        return false;
}

VSV_t FileParser::get_VSV(uint8_t *p_vsv, uint8_t index)
{
    VSV_t vsv_type;

    p_vsv = p_vsv + (index >> 1); // смещение по полю VSV, в байт помещается два поля VSV

    if ( (index & 0x1) == 0) // чет - берем младшую тетраду
    {
        vsv_type = (VSV_t)((*p_vsv) & VSV_MSK);
    }
    else // нечет - берем старшую тетраду
    {
        vsv_type = (VSV_t)(((*p_vsv) & (VSV_MSK << VSV_BITS)) >> VSV_BITS);
    }

    return vsv_type;
}
