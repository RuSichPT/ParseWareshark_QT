#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QObject>
#include <QVector>
#include <QFile>
#include <QTimer>
#include "external_files/MAC/Inc/MAC_pkts_format.h"  //Pkt_Hdr, VSV_t ,Pkt_type_Hdr и тд

#define VSV_BITS                    4
#define VSV_MSK                     ((1 << VSV_BITS) - 1)
#define PKT_TYPE_TYPE               PktType

class FileParser : public QObject
{
    Q_OBJECT

private:
    static const int LEN_SYNHR              {68};

    static const int UPDATE_PBAR_MSEC       {500};

    static const int START_OFFSET           {24};
    static const int FRAMES_OFFSET          {16};
    static const int LEN_FRAME_OFFSET       {8};
    static const int TYPE_FRAME_OFFSET      {12};
    static const int UCOS_OFFSET            {42};
    static const int PSPD_OFFSET            {54};
    static const int SIZE_BYTE_BITS         {8};
    static const int SIZE_INT_BYTES         {4};
    static const int SIZE_SHORT_BYTES       {2};
    static const int SIZE_MATCH             {256};

public:

    struct MissedPkt_t
    {
        uint16_t                destAddr;
        uint16_t                srcAddr;
        uint8_t                 pktNumber;
        int                     pktFrameNumber;
        int                     beacFrameNumber;
        int                     cycle;
    };

    struct RepeatedPkt_t
    {
        uint16_t                srcAddr;
        uint8_t                 pktNumber;
        int                     pktFrameNumber;
    };

    struct Counter_t
    {
        uint16_t                addr;
        uint8_t                 currentPktNumber;
        int                     currentCycle;
        int                     matchPktToFrame[SIZE_MATCH];
    };

public:
    explicit FileParser(QObject *parent = nullptr);
    ~FileParser();

    Counter_t* getCounter(uint16_t addr);
    void setCounter(uint16_t addr, uint8_t pktNumber, int frameNumber);
    Counter_t* createCounter(uint16_t addr, uint8_t pktNumber);

    void updateCurrentNumber(Counter_t *counter, uint8_t pktNumber);

    MissedPkt_t *getMissedPkt(uint16_t srcAddr, uint8_t pktNumber, int cycle);
    void setMissedPkt(uint16_t destAddr, uint16_t srcAddr, uint8_t pktNumber, int beacFrameNum, int pktFrameNum, int cycle);

    void setRepeatedPkt(uint16_t srcAddr, uint8_t pktNumber, int pktFrameNum);

    QVector<MissedPkt_t> &getMissedPkts(){return m_missedPkts;}
    QVector<RepeatedPkt_t> &getRepeatedPkts(){return m_repeatedPkts;}

    void readFile(QFile &file);

signals:
    void timeout(uint64_t readBytes);

private:
    QTimer                  *m_timer;

    QVector<Counter_t>      m_counters;
    QVector<MissedPkt_t>    m_missedPkts;
    QVector<RepeatedPkt_t>  m_repeatedPkts;
    uint64_t                m_readBytes;

    void parseFrame(const QByteArray &frame, int numFrame);
    void parseService(Pkt_Hdr *PktData, int numFrame);
    void parseBeacon(Pkt_Hdr *PktData, int numFrame);
    int bytesToInt(const QByteArray &bytes, int first, int sizeInt);
    bool CompNum8(uint8_t Num1, uint8_t Num2);
    VSV_t get_VSV(uint8_t *p_vsv, uint8_t index);
};

#endif // FILEPARSER_H
