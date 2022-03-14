#include "inc/fileparser.h"
#include "inc/mywidget.h"
#include <QDebug>

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

FileParser::counter_t *FileParser::getCounter(uint16_t addr)
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
    counter_t *counter = getCounter(addr);

    if(counter == nullptr)
    {
        counter_t tmp;
        tmp.addr = addr;
        tmp.currentpktNumber = pktNumber;
        m_counters.append(tmp);
    }
    else
    {
        if(CompNum8(pktNumber, counter->currentpktNumber))
        {
            counter->currentpktNumber = pktNumber;
        }
    }

    counter = getCounter(addr);
    if (pktNumber < sizeMatch)
    {
        counter->matchPktToFrame[pktNumber] = frameNumber;
    }
}

FileParser::missedPkt_t *FileParser::getMissedPkt(uint16_t addr)
{
    if(!m_missedPkts.isEmpty())
    {
        for (int var = 0; var < m_missedPkts.size(); ++var)
        {
            if(m_missedPkts.at(var).addr == addr)
            {
                return &m_missedPkts[var];
            }
        }
    }

    return nullptr;
}

void FileParser::setMissedPkt(uint16_t addr, uint16_t addrFrom, uint8_t pktNumber, int beacFrameNum, int pktFrameNum)
{
    missedPkt_t *missedPkt = getMissedPkt(addr);

    if(missedPkt == nullptr)
    {
        missedPkt_t tmp;
        mapAddrPkt_t tmpMap;
        tmp.addr = addr;
        tmpMap.addrFrom = addrFrom;
        tmpMap.pktNumber = pktNumber;
        tmpMap.pktFrameNumber = pktFrameNum;
        tmpMap.beacFrameNumber = beacFrameNum;
        tmp.pktNumbers.append(tmpMap);
        m_missedPkts.append(tmp);
    }
    else
    {
        mapAddrPkt_t tmpMap;
        tmpMap.addrFrom = addrFrom;
        tmpMap.pktNumber = pktNumber;
        tmpMap.pktFrameNumber = pktFrameNum;
        tmpMap.beacFrameNumber = beacFrameNum;
        missedPkt->pktNumbers.append(tmpMap);
    }
}

void FileParser::readFile(QFile &file)
{
    if(file.exists())
    {
        QByteArray frameOf, frame;
        int lengthFrame;
        int numFrame {1};

        // Запускаем таймер
        m_timer->start(updatePBarMsec);
        file.read(startOffset);
        m_readBytes += startOffset;
        while(!file.atEnd())
        {
            if (m_timer->remainingTime() != 0)
            {
                frameOf = file.read(framesOffset);
                // Узнаем длину фрейма
                lengthFrame = bytesToInt(frameOf, lengthFramOffs, sizeIntBytes);
                frame = file.read(lengthFrame);
                parseFrame(frame, numFrame);
                numFrame++;
                m_readBytes += framesOffset + lengthFrame;
            }
            else
            {
                emit timeout(m_readBytes);
                m_timer->start(updatePBarMsec);
            }
        }

        emit timeout(m_readBytes);
        m_timer->stop();
    }
}

void FileParser::parseFrame(const QByteArray &frame, int numFrame)
{
    // Узнаем тип
    int typeFrame = bytesToInt(frame, typeFrameOffs, sizeShortBytes);
    if (typeFrame == PP_HTONS(ETHTYPE_IP))
    {
        // Узнаем Modem_CMD_Name
        int modemCMDName = bytesToInt(frame, UCOSOffset, sizeShortBytes);
        if (modemCMDName == UCOS_CMD_TXDATA)
        {
            // Исключаем пакет синхронизации
            if (frame.size() != lengthSynhr)
            {
                const char *pFrame;
                Pkt_Hdr *PktData;

                pFrame = frame.data();
                pFrame += PSPDOffset;
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

    if(PktData->DestAddr == SOFT_ADDR_BRDCST)
    {
        do //Piggy-back по PKTdata->pData_LenToNext - по разным типам пакетов внутри одного пакета с хедером
        {
            LenToNext_field = type_Hdr->LenToNext;

            // Смотрим все служебные пакеты и запоминаем номер последнего пакета от абонента
            if ( (PktData->Data_type == PKT_TYPE_CTS)
                 || (PktData->Data_type == PKT_TYPE_RTS)
                 || (PktData->Data_type == PKT_TYPE_ACK)
                 || (PktData->Data_type == PKT_TYPE_NACK)
                 || (PktData->Data_type == PKT_TYPE_DATA_MAP)
                 || (PktData->Data_type == PKT_TYPE_DATA_MAP_REQUEST)
                 )
            {

                Pkt_Service *Service = (Pkt_Service *)(type_Hdr);

                setCounter(PktData->SrcAddr, Service->pkt_number, numFrame);
            }
            type_Hdr = (Pkt_type_Hdr*) ( ((uint8_t*)(&(type_Hdr->LenToNext))) + sizeof(PKT_LENTONEXT_TYPE) + LenToNext_field );
        }
        while(LenToNext_field != 0);
    }
}

void FileParser::parseBeacon(Pkt_Hdr *PktData, int beacFrameNum)
{
    if(PktData->DestAddr == SOFT_ADDR_BRDCST)
    {
        // Смотрим пропуски пакетов
        if ( PktData->Data_type == PKT_TYPE_BEACON)
        {
            Pkt_Beacon *Beacon = (Pkt_Beacon *)&(PktData->Data_type);

            for (int var = 0; var < MAX_RADIO_CONNECTIONS; ++var)
            {
                counter_t* counter = getCounter(Beacon->AS[var].addr);
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
                            if (Beacon->last_heard_pkt_number[var] != counter->currentpktNumber)
                            {
                                int pktNumber = Beacon->last_heard_pkt_number[var] + 1;
                                int pktFrameNum = counter->matchPktToFrame[pktNumber];
                                setMissedPkt(PktData->SrcAddr, counter->addr, pktNumber, beacFrameNum, pktFrameNum);
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
        result = result | (tmp << i*sizeByteBits);
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
