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

    enum class SendPhase
    {
        WaitTalkTurnaround,
        WaitTalkAttentionRelease,
        WaitTalkAcknowledgeHold,
        WaitTalkListenerReady,
        WaitReady,
        WaitEOIStart,
        WaitEOIAcknowledge,
        WaitEOIRelease,
        DriveBit,
        WaitClockLow,
        WaitClockHigh,
        WaitByteAcknowledge,
        WaitAcknowledgeRelease
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

    explicit IECProtocol(IECBus* ptrBus, quint8 deviceNumber, IECBus::ParticipantId participantId);

    quint8 deviceNumber() const             { return m_deviceNumber; }
    State state() const                     { return m_state; }
    Mode mode() const                       { return m_mode; }
    quint8 secondaryAddress() const         { return m_secondaryAddress; }

    void reset();
    void clock();

    bool hasReceivedByte() const;
    ReceivedByte takeReceivedByte();
    bool takeListenEnded();

    bool startSending(const QByteArray& data);

private:
    void processAttention();
    void processReceivedByte(quint8 value, bool attention);
    void beginNextByte();
    void releaseBus();

    void clockSend();
    void prepareTransmitByte();
    void finishSending();

    bool lineHigh(IECBus::Line line) const;
    void setLineLow(IECBus::Line line, bool low);

    void resetSendDiagnostics();

private:
    static constexpr quint32 EOI_TIMEOUT_CYCLES = 200;
    static constexpr quint32 EOI_ACK_CYCLES = 80;
    static constexpr quint32 BYTE_ACK_CYCLES = 100;
    static constexpr quint32 SEND_BIT_LOW_CYCLES = 60;
    static constexpr quint32 SEND_BIT_HIGH_CYCLES = 60;
    static constexpr quint32 TALK_ATTENTION_RELEASE_CYCLES = 30;
    static constexpr quint32 TALK_ATTENTION_ACK_HOLD_CYCLES = 80;
    static constexpr quint32 BETWEEN_BYTES_CYCLES = 103;

    quint8 m_deviceNumber = 0;
    IECBus* m_ptrBus = nullptr;
    IECBus::ParticipantId m_participantId = 0;

    State m_state = State::Idle;
    Mode m_mode = Mode::None;

    quint8 m_secondaryAddress = 0;
    quint8 m_receivedByte = 0;
    quint8 m_receivedBits = 0;
    bool m_listenEnded = false;

    quint32 m_stateCycles = 0;

    bool m_previousAttentionHigh = true;
    bool m_previousClockHigh = true;
    bool m_endOfInformation = false;

    // Merkt sich, ob während der aktuellen ATN-Phase
    // CLK bereits auf Low gewesen ist.
    bool m_attentionClockLowSeen = false;

    QQueue<ReceivedByte> m_receivedBytes;

    SendPhase m_sendPhase = SendPhase::WaitReady;
    QByteArray m_transmitData;
    qsizetype m_transmitIndex = 0;
    quint8 m_transmitBit = 0;

    // Diagnose eines länger andauernden Stillstands beim Senden.
    quint32 m_sendNoProgressCycles = 0;
    SendPhase m_lastDiagnosticPhase = SendPhase::WaitReady;
    qsizetype m_lastDiagnosticIndex = -1;
    quint8 m_lastDiagnosticBit = 255;
};
