#pragma once

#include <QtGlobal>

class MOS6526Port;


class MOS6526
{
public:
    explicit MOS6526();
    virtual ~MOS6526();

    // Getter
    quint8 readRegister(quint8 address);
    quint8 portAPins() const                                { return (m_portA & m_dataDirectionA) | (m_portAInputs & ~m_dataDirectionA); }
    quint8 portBPins() const                                { return (m_portB & m_dataDirectionB) | (m_portBInputs & ~m_dataDirectionB); }
    bool irq() const                                        { return (m_interruptStatus & m_interruptMask) != 0x00; }
    quint8 portAOutput() const                              { return m_portA | static_cast<quint8>(~m_dataDirectionA); }
    quint8 portBOutput() const                              { return m_portB | static_cast<quint8>(~m_dataDirectionB); }

    // Setter
    void setPort(MOS6526Port* ptrPort)                      { m_ptrPort = ptrPort; }
    void writeRegister(quint8 address, quint8 value);
    void setPortAInputs(quint8 value)                       { m_portAInputs = value; }
    void setPortBInputs(quint8 value)                       { m_portBInputs = value; }

    // Operations
    void clock();

private:
    MOS6526Port* m_ptrPort = nullptr;

    quint8 m_portA = 0x00;
    quint8 m_dataDirectionA = 0x00;
    quint8 m_portAInputs = 0xFF;

    quint8 m_portB = 0x00;
    quint8 m_dataDirectionB = 0x00;
    quint8 m_portBInputs = 0xFF;

    quint16 m_timerALatch = 0x0000;
    quint16 m_timerACounter = 0x0000;
    quint8 m_controlRegisterA = 0x00;
    quint8 m_controlRegisterB = 0x00;

    quint8 m_interruptStatus = 0x00;
    quint8 m_interruptMask = 0x00;

    quint16 m_unimplementedReadReported = 0x0000;
    quint16 m_unimplementedWriteReported = 0x0000;
};
