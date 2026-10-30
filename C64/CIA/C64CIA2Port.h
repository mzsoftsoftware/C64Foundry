#pragma once

#include "MOS6526Port.h"
#include "C64/IEC/IECBus.h"


class C64CIA2Port : public MOS6526Port
{
public:
    explicit C64CIA2Port();
    virtual ~C64CIA2Port() override;

    // Setter
    void setIECBus(IECBus* ptrIECBus);

    // Operations
    quint8 portAInputs(quint8 portBOutput) const override;
    quint8 portBInputs(quint8 portAOutput) const override;

    void portAOutputChanged(quint8 portA, quint8 dataDirectionA) override;

private:
    IECBus* m_ptrIECBus = nullptr;
    IECBus::ParticipantId m_participantId = 0;

    mutable bool m_hasPreviousSample = false;
    mutable bool m_previousSampleClockHigh = true;
    mutable bool m_previousSampleDataHigh = true;
};
