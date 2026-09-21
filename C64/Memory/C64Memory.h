#pragma once

#include <QtGlobal>

class RAM;
class BasicROM;
class KernalROM;
class CharROM;
class ColorRAM;


class C64Memory
{
public:
    explicit C64Memory();
    virtual ~C64Memory();

    // Operations RAM
    quint8 readRAM(const quint16 address) const;
    void writeRAM(const quint16 address, const quint8 value);

    // Operations ROM
    quint8 readBasicROM(const quint16 address) const;
    quint8 readKernalROM(const quint16 address) const;
    quint8 readCharacterROM(const quint16 address) const;

    // Operations COLOR-RAM
    quint8 readColorRAM(const quint16 address) const;
    void writeColorRAM(const quint16 address, const quint8 value);

private:
    RAM*        m_ptrRAM = nullptr;
    BasicROM*   m_ptrBasicROM = nullptr;
    KernalROM*  m_ptrKernalROM = nullptr;
    CharROM*    m_ptrCharROM = nullptr;
    ColorRAM*   m_ptrColorRAM = nullptr;
};
