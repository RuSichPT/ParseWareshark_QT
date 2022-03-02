#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QObject>
#include <stdint.h>                 // uint16_t, uint8_t
#include "inc/MAC_pkts_format.h"    // all
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
    static const int lengthSynhr            {68};
    static const int IPv4                   {0x0008};

    static const int updatePBarMsec         {500};

    static const int startOffset            {24};
    static const int framesOffset           {16};
    static const int lengthFramOffs         {8};
    static const int typeFrameOffs          {12};
    static const int UCOSOffset             {42};
    static const int PSPDOffset             {54};
    static const int sizeByteBits           {8};
    static const int sizeIntBytes           {4};
    static const int sizeShortBytes         {2};
    static const int sizeMatch              {255};

public:

    struct mapAddrPkt_t
    {
        uint16_t                addrFrom;
        uint8_t                 pktNumber;
        int                     pktFrameNumber;
        int                     beacFrameNumber;
    };

    struct missedPkt_t
    {
        uint16_t                addr;
        QVector<mapAddrPkt_t>   pktNumbers;
    };


    struct counter_t
    {
        uint16_t                addr;
        uint8_t                 currentpktNumber;
        int                     matchPktToFrame[sizeMatch];
    };

public:
    explicit FileParser(QObject *parent = nullptr);
    ~FileParser();

    counter_t *getCounter(uint16_t addr);
    void setCounter(uint16_t addr, uint8_t pktNumber, int frameNumber);

    missedPkt_t *getMissedPkt(uint16_t addr);
    void setMissedPkt(uint16_t addr, uint16_t addrFrom, uint8_t pktNumber, int beacFrameNum, int pktFrameNum);

    QVector<missedPkt_t> &getMissedPkts(){return m_missedPkts;}

    void readFile(QFile &file);

signals:
    void timeout(uint64_t readBytes);

private:
    QTimer                  *m_timer;

    QVector<counter_t>      m_counters;
    QVector<missedPkt_t>    m_missedPkts;
    uint64_t                m_readBytes;

    void parseFrame(const QByteArray &frame, int numFrame);
    void parseService(Pkt_Hdr *PktData, int numFrame);
    void parseBeacon(Pkt_Hdr *PktData, int numFrame);
    int bytesToInt(const QByteArray &bytes, int first, int sizeInt);
    bool CompNum8(uint8_t Num1, uint8_t Num2);
    VSV_t get_VSV(uint8_t *p_vsv, uint8_t index);
};

#endif // FILEPARSER_H
