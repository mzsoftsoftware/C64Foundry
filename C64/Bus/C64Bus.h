#pragma once

#include <QtGlobal>

class C64Memory;


class C64Bus
{
public:
    explicit C64Bus();
    virtual ~C64Bus();

    // Setter
    void setMemory(C64Memory* ptrMemory);
    //void setVICII(VICII* ptrVicII);
    //void setSID(SID* ptrSid);
    //void setCIA1(CIA* ptrCia1);
    //void setCIA2(CIA* ptrCia2);

    // Operations
    quint8 read(quint16 address) const;
    void write(quint16 address, quint8 value);

private:
    C64Memory* m_ptrMemory;
};
