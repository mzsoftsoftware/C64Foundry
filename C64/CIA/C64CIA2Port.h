#pragma once

#include "MOS6526Port.h"
#include "C64/IEC/IECBus.h"


class C64CIA2Port : public MOS6526Port
{
public:
    explicit C64CIA2Port(IECBus* ptrBus);
    virtual ~C64CIA2Port() override;

    quint8 portAInputs(quint8 portBOutput) const override;
    quint8 portBInputs(quint8 portAOutput) const override;

    void portAOutputChanged(quint8 portA, quint8 dataDirectionA) override;

private:
    IECBus* m_ptrBus = nullptr;
    IECBus::ParticipantId m_participantId = 0;
};
