
#pragma once

#include <QtGlobal>

#include "IECBus.h"


class IECProtocol
{
public:
    enum class State
    {
        Idle,
        Attention,
        Receive,
        Acknowledge,
        Send
    };

    enum class Mode
    {
        None,
        Listen,
        Talk
    };

    explicit IECProtocol(quint8 deviceNumber, IECBus* ptrBus, IECBus::ParticipantId participantId);

    quint8 deviceNumber() const { return m_deviceNumber; }
    State state() const { return m_state; }
    Mode mode() const { return m_mode; }
    quint8 secondaryAddress() const { return m_secondaryAddress; }

    void reset();
    void clock();

private:
    void processAttention();
    void processReceivedByte(quint8 value, bool attention);

    bool lineHigh(IECBus::Line line) const;
    void setLineLow(IECBus::Line line, bool low);

private:
    quint8 m_deviceNumber = 0;
    IECBus* m_ptrBus = nullptr;
    IECBus::ParticipantId m_participantId = 0;

    State m_state = State::Idle;
    Mode m_mode = Mode::None;

    quint8 m_secondaryAddress = 0;
    quint8 m_receivedByte = 0;
    quint8 m_receivedBits = 0;

    bool m_previousAttentionHigh = true;
    bool m_previousClockHigh = true;
};
