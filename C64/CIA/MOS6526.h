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
    quint8 portAPins() const                                { return (m_portA & m_dataDirectionA) | (m_portAInputs & static_cast<quint8>(~m_dataDirectionA)); }
    quint8 portAOutput() const                              { return m_portA | static_cast<quint8>(~m_dataDirectionA); }
    quint8 portBPins() const                                { return portBValue(m_portBInputs); }
    quint8 portBOutput() const                              { return portBValue(0xFF); }
    bool irq() const                                        { return (m_interruptStatus & m_interruptMask) != 0x00; }
    bool serialPortInput() const                            { return m_serialPortInput; }
    bool serialPortOutput() const                           { return m_serialPortOutput; }
    bool serialClockOutput() const                          { return m_serialClockOutput; }

    // Setter
    void setPort(MOS6526Port* ptrPort)                      { m_ptrPort = ptrPort; }
    void writeRegister(quint8 address, quint8 value);
    void setPortAInputs(quint8 value)                       { m_portAInputs = value; }
    void setPortBInputs(quint8 value)                       { m_portBInputs = value; }
    void setCNT(bool level);
    void setTODDivider(quint8 divider);
    void clockTODPulse();
    void setSP(bool level)                                  { m_serialPortInput = level; }
    void setFLAG(bool level);

    // Operations
    void initialize();
    void reset();
    void clock();

private:
    bool clockTimerA();
    bool clockTimerB();
    void timerAUnderflow();
    void incrementTOD();
    void checkTODAlarm();
    void serialPortOutputTimerAUnderflow();
    void setSerialOutput(bool dataLevel, bool clockLevel);
    quint8 portBValue(quint8 inputs) const;
    void timerBUnderflow();

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
    quint16 m_timerBLatch = 0x0000;
    quint16 m_timerBCounter = 0x0000;

    quint8 m_controlRegisterA = 0x00;
    quint8 m_controlRegisterB = 0x00;

    quint8 m_interruptStatus = 0x00;
    quint8 m_interruptMask = 0x00;

    quint16 m_unimplementedReadReported = 0x0000;
    quint16 m_unimplementedWriteReported = 0x0000;

    bool m_cnt = true;

    bool m_serialPortInput = true;
    bool m_serialPortOutput = true;

    quint8 m_tod[4] = { 0x00, 0x00, 0x00, 0x00 };
    quint8 m_todAlarm[4] = { 0x00, 0x00, 0x00, 0x00 };
    quint8 m_todReadLatch[4] = { 0x00, 0x00, 0x00, 0x00 };

    quint8 m_todDivider = 5;
    quint8 m_todDividerCounter = 0;

    bool m_todStopped = false;
    bool m_todReadLatched = false;

    quint8 m_serialDataRegister = 0x00;
    quint8 m_serialBitCount = 0;

    quint8 m_serialShiftRegister = 0x00;
    bool m_serialClockOutput = true;
    bool m_serialOutputPending = false;
    bool m_serialOutputActive = false;
    bool m_serialOutputClockPhase = false;

    bool m_flag = true;
    bool m_timerAOutput = false;
    bool m_timerBOutput = false;

    bool m_timerAPulse = false;
    bool m_timerBPulse = false;
};
