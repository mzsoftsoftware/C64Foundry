#pragma once

#include <QtGlobal>


class MOS6526
{
public:
    explicit MOS6526();
    virtual ~MOS6526();

    // Getter
    quint8 readRegister(quint8 address) const;

    // Setter
    void writeRegister(quint8 address, quint8 value);
    void setPortAInputs(quint8 value)                       { m_portAInputs = value; }
    void setPortBInputs(quint8 value)                       { m_portBInputs = value; }

private:
    quint8 m_portA = 0x00;
    quint8 m_dataDirectionA = 0x00;
    quint8 m_portAInputs = 0xFF;

    quint8 m_portB = 0x00;
    quint8 m_dataDirectionB = 0x00;
    quint8 m_portBInputs = 0xFF;
};
