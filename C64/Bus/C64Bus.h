#pragma once

#include <QtGlobal>

class C64Memory;
class VICII;

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
    bool aec() const                            { return m_aec; }
    bool cpuDrivesDataBus() const               { return m_cpuDrivesDataBus; }
    quint8 dataBusValue() const                 { return m_dataBusValue; }
    quint8 cpuPortLines() const                 { return m_cpuPortLines; }
    MemorySource memorySource(quint16 address) const;

    AccessType lastAccessType() const           { return m_lastAccessType; }
    quint16 lastAccessAddress() const           { return m_lastAccessAddress; }
    quint8 lastAccessValue() const              { return m_lastAccessValue; }
    quint8 accessCount() const                  { return m_accessCount; }

    // Setter
    void setAEC(bool high)                      { m_aec = high; }
    void setDataBusValue(quint8 value)          { m_dataBusValue = value; }
    void setCpuPortLines(const quint8 lines);

    void setMemory(C64Memory* ptrMemory);
    void setVICII(VICII* ptrVicII);
    //void setSID(SID* ptrSid);
    //void setCIA1(CIA* ptrCia1);
    //void setCIA2(CIA* ptrCia2);

    // Operations
    void clock();

    quint8 read(quint16 address);
    void write(quint16 address, quint8 value);

    void readCycle(quint16 address);
    void writeCycle(quint16 address);

    quint8 readVIC(quint16 address);


private:
    C64Memory* m_ptrMemory = nullptr;
    VICII* m_ptrVICII = nullptr;

    bool m_aec = true;
    bool m_cpuDrivesDataBus = false;
    quint8 m_dataBusValue = 0x00;
    quint8 m_cpuPortLines = 0x07;

    const MemorySource* m_ptrMemoryMap = nullptr;

    AccessType m_lastAccessType = AccessType::None;
    quint16 m_lastAccessAddress = 0x0000;
    quint8 m_lastAccessValue = 0x00;
    quint8 m_accessCount = 0;

private:
    static constexpr MemorySource s_memoryMaps[8][16] =
        {
            //
            // 000: LORAM=0, HIRAM=0, CHAREN=0
            //
            {
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM,
                MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM
            },

            //
            // 001: LORAM=1, HIRAM=0, CHAREN=0
            //
            {
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM,
                MemorySource::CharacterROM,
                MemorySource::RAM, MemorySource::RAM
            },

            //
            // 010: LORAM=0, HIRAM=1, CHAREN=0
            //
            {
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM,
                MemorySource::CharacterROM,
                MemorySource::KernalROM, MemorySource::KernalROM
            },

            //
            // 011: LORAM=1, HIRAM=1, CHAREN=0
            //
            {
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::BasicROM, MemorySource::BasicROM,
                MemorySource::RAM,
                MemorySource::CharacterROM,
                MemorySource::KernalROM, MemorySource::KernalROM
            },

            //
            // 100: LORAM=0, HIRAM=0, CHAREN=1
            //
            {
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM,
                MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM
            },

            //
            // 101: LORAM=1, HIRAM=0, CHAREN=1
            //
            {
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM,
                MemorySource::IO,
                MemorySource::RAM, MemorySource::RAM
            },

            //
            // 110: LORAM=0, HIRAM=1, CHAREN=1
            //
            {
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM,
                MemorySource::IO,
                MemorySource::KernalROM, MemorySource::KernalROM
            },

            //
            // 111: LORAM=1, HIRAM=1, CHAREN=1
            //
            {
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::RAM, MemorySource::RAM,
                MemorySource::BasicROM, MemorySource::BasicROM,
                MemorySource::RAM,
                MemorySource::IO,
                MemorySource::KernalROM, MemorySource::KernalROM
            }
    };
};
