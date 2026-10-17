#pragma once

#include <QtGlobal>


class MOS6526Port
{
public:
    virtual ~MOS6526Port() = default;

    virtual quint8 portAInputs(quint8 portBOutput) const = 0;
    virtual quint8 portBInputs(quint8 portAOutput) const = 0;

    virtual void portAOutputChanged(quint8 portA, quint8 dataDirectionA)
    {
        Q_UNUSED(portA);
        Q_UNUSED(dataDirectionA);
    }
};
