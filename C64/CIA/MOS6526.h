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

private:
    quint8 m_portA = 0x00;
};
