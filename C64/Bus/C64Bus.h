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

    explicit C64Bus();
    virtual ~C64Bus();

    // Setter
    void setMemory(C64Memory* ptrMemory);
    //void setVICII(VICII* ptrVicII);
    //void setSID(SID* ptrSid);
    //void setCIA1(CIA* ptrCia1);
    //void setCIA2(CIA* ptrCia2);

    // Operations
    void clock();
    quint8 read(quint16 address);
    void write(quint16 address, quint8 value);

    AccessType lastAccessType() const           { return m_lastAccessType; }
    quint16 lastAccessAddress() const           { return m_lastAccessAddress; }
    quint8 lastAccessValue() const              { return m_lastAccessValue; }
    quint8 accessCount() const                  { return m_accessCount; }

private:
    C64Memory* m_ptrMemory;

    AccessType m_lastAccessType = AccessType::None;
    quint16 m_lastAccessAddress = 0x0000;
    quint8 m_lastAccessValue = 0x00;
    quint8 m_accessCount = 0;
};
