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
        WaitTalkAcknowledgeHold,
        WaitTalkListenerReady,
        WaitInitialDataStart,
        WaitReady,
        WaitEOIStart,
        WaitEOIAcknowledge,
        WaitEOIRelease,
        DriveBit,
        WaitClockLow,
        WaitClockHigh,
        WaitFinalBitDataHold,
        WaitByteAcknowledge,
        WaitAcknowledgeHold,
        WaitAcknowledgeRelease,
        WaitBetweenBytes
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

private:
    // IEC timing values in C64 clock cycles.
    // Values are rounded up to satisfy PAL and NTSC.

    // Listener receiving from an external talker.
    static constexpr quint32 EOI_TIMEOUT_CYCLES = 205;
    static constexpr quint32 EOI_ACK_CYCLES = 82;
    static constexpr quint32 BYTE_ACK_CYCLES = 100;
    static constexpr quint32 TALK_LISTENER_READY_DELAY_CYCLES = 62;

    // External talker transmitting to the C64.
    static constexpr quint32 SEND_BIT_SETUP_CYCLES = 62;
    static constexpr quint32 SEND_FINAL_BIT_DATA_HOLD_CYCLES = 62;
    static constexpr quint32 SEND_DATA_VALID_CYCLES = 62;
    static constexpr quint32 SEND_BYTE_ACK_HOLD_CYCLES = 62;

    // Handshake timing.
    static constexpr quint32 BETWEEN_BYTES_CYCLES = 103;
    static constexpr quint32 FRAME_HANDSHAKE_TIMEOUT_CYCLES = 1023;

    // Talk attention turnaround.
    static constexpr quint32 TALK_ATTENTION_ACK_HOLD_CYCLES = 82;

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

    bool m_previousSendClockHigh = true;
    quint8 m_transmitClockRiseCount = 0;
};
