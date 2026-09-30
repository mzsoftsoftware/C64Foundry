#pragma once

#include <QtGlobal>

class C64Memory;

class C64Bus
{
public:
    enum class AccessType
    {
        None,
        Read,
        Write
    };
    enum class MemorySource
    {
        RAM,
        BasicROM,
        KernalROM,
        CharacterROM,
        IO
    };

    explicit C64Bus();
    virtual ~C64Bus();

    // Getter
    bool cpuDrivesDataBus() const               { return m_cpuDrivesDataBus; }
    quint8 dataBusValue() const                 { return m_dataBusValue; }
    quint8 cpuPortLines() const                 { return m_cpuPortLines; }
    MemorySource memorySource(quint16 address) const;

    AccessType lastAccessType() const           { return m_lastAccessType; }
    quint16 lastAccessAddress() const           { return m_lastAccessAddress; }
    quint8 lastAccessValue() const              { return m_lastAccessValue; }
    quint8 accessCount() const                  { return m_accessCount; }

    // Setter
    void setDataBusValue(quint8 value)          { m_dataBusValue = value; }
    void setCpuPortLines(const quint8 lines);

    void setMemory(C64Memory* ptrMemory);
    //void setVICII(VICII* ptrVicII);
    //void setSID(SID* ptrSid);
    //void setCIA1(CIA* ptrCia1);
    //void setCIA2(CIA* ptrCia2);

    // Operations
    void clock();

    quint8 read(quint16 address);
    void write(quint16 address, quint8 value);

    void readCycle(quint16 address);
    void writeCycle(quint16 address);


private:
    C64Memory* m_ptrMemory = nullptr;

    bool m_cpuDrivesDataBus = false;
    quint8 m_dataBusValue = 0x00;
    quint8 m_cpuPortLines = 0x07;

    MemorySource m_basicSource;
    MemorySource m_ioSource;
    MemorySource m_kernalSource;

    AccessType m_lastAccessType = AccessType::None;
    quint16 m_lastAccessAddress = 0x0000;
    quint8 m_lastAccessValue = 0x00;
    quint8 m_accessCount = 0;

private:
    static constexpr MemorySource s_basicMapping[8] =
    {
            MemorySource::RAM,       // 000
            MemorySource::RAM,       // 001
            MemorySource::RAM,       // 010
            MemorySource::BasicROM,  // 011
            MemorySource::RAM,       // 100
            MemorySource::RAM,       // 101
            MemorySource::RAM,       // 110
            MemorySource::BasicROM   // 111
    };

    static constexpr MemorySource s_ioMapping[8] =
    {
            MemorySource::RAM,           // 000
            MemorySource::CharacterROM,  // 001
            MemorySource::CharacterROM,  // 010
            MemorySource::CharacterROM,  // 011
            MemorySource::RAM,           // 100
            MemorySource::IO,            // 101
            MemorySource::IO,            // 110
            MemorySource::IO             // 111
    };

    static constexpr MemorySource s_kernalMapping[8] =
    {
            MemorySource::RAM,        // 000
            MemorySource::RAM,        // 001
            MemorySource::KernalROM,  // 010
            MemorySource::KernalROM,  // 011
            MemorySource::RAM,        // 100
            MemorySource::RAM,        // 101
            MemorySource::KernalROM,  // 110
            MemorySource::KernalROM   // 111
    };
};
