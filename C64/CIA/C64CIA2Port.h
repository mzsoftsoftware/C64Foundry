#pragma once

#include "MOS6526Port.h"
#include "C64/IEC/IECBus.h"
#include "C64/UserPort/UserPortBus.h"


class C64CIA2Port : public MOS6526Port
{
public:
    explicit C64CIA2Port();
    virtual ~C64CIA2Port() override;

    // Setter
    void setIECBus(IECBus* ptrIECBus);
    void setUserPortBus(UserPortBus* ptrUserPortBus);

    // Operations
    quint8 portAInputs(quint8 portBOutput) const override;
    quint8 portBInputs(quint8 portAOutput) const override;

    void portAOutputChanged(quint8 portA, quint8 dataDirectionA) override;
    void serialPortOutputChanged(bool serialData, bool serialClock) override;

    // Getter
    bool serialDataOutput() const     { return m_serialDataOutput; }
    bool serialClockOutput() const    { return m_serialClockOutput; }

private:
    IECBus* m_ptrIECBus = nullptr;
    IECBus::ParticipantId m_participantId = 0;

    UserPortBus* m_ptrUserPortBus = nullptr;
    UserPortBus::ParticipantId m_userPortParticipantId = 0;

    mutable quint16 m_portAReadLogCount = 0;
    bool m_serialDataOutput = true;
    bool m_serialClockOutput = true;
};
