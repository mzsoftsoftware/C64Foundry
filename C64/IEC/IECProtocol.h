#pragma once

#include <QQueue>
#include <QtGlobal>

#include "IECBus.h"


class IECProtocol
{
public:
    enum class State
    {
        Idle,
        Attention,
        WaitClockHigh,
        WaitFirstClockLow,
        ReceiveBits,
        WaitByteEnd,
        Acknowledge,
        WaitNextClockHigh,
        AcknowledgeEOI,
        WaitEOIRelease,
        Send
    };

    enum class Mode
    {
        None,
        Listen,
        Talk
    };

    struct ReceivedByte
    {
        quint8 value = 0;
        quint8 secondaryAddress = 0;
        bool eoi = false;
    };

    explicit IECProtocol(quint8 deviceNumber, IECBus* ptrBus, IECBus::ParticipantId participantId);

    quint8 deviceNumber() const             { return m_deviceNumber; }
    State state() const                     { return m_state; }
    Mode mode() const                       { return m_mode; }
    quint8 secondaryAddress() const         { return m_secondaryAddress; }

    void reset();
    void clock();

    bool hasReceivedByte() const;
    ReceivedByte takeReceivedByte();

private:
    void processAttention();
    void processReceivedByte(quint8 value, bool attention);
    void beginNextByte();
    void releaseBus();

    bool lineHigh(IECBus::Line line) const;
    void setLineLow(IECBus::Line line, bool low);

private:
    static constexpr quint32 EOI_TIMEOUT_CYCLES = 200;
    static constexpr quint32 EOI_ACK_CYCLES = 80;
    static constexpr quint32 BYTE_ACK_CYCLES = 100;

    quint8 m_deviceNumber = 0;
    IECBus* m_ptrBus = nullptr;
    IECBus::ParticipantId m_participantId = 0;

    State m_state = State::Idle;
    Mode m_mode = Mode::None;

    quint8 m_secondaryAddress = 0;
    quint8 m_receivedByte = 0;
    quint8 m_receivedBits = 0;

    quint32 m_stateCycles = 0;

    bool m_previousAttentionHigh = true;
    bool m_previousClockHigh = true;
    bool m_endOfInformation = false;

    // Merkt sich, ob während der aktuellen ATN-Phase
    // CLK bereits auf Low gewesen ist.
    bool m_attentionClockLowSeen = false;

    QQueue<ReceivedByte> m_receivedBytes;
};
