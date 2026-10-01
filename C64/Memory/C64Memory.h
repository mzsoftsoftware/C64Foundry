#pragma once

#include <QtGlobal>

class RAM;
class ROM;
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
    bool loadBasicROM(const QByteArray& data);
    bool loadBasicROM(const QString& fileName);
    bool loadKernalROM(const QByteArray& data);
    bool loadKernalROM(const QString& fileName);
    bool loadCharacterROM(const QByteArray& data);
    bool loadCharacterROM(const QString& fileName);

    quint8 readBasicROM(const quint16 address) const;
    quint8 readKernalROM(const quint16 address) const;
    quint8 readCharacterROM(const quint16 address) const;

    // Operations COLOR-RAM
    quint8 readColorRAM(const quint16 address) const;
    void writeColorRAM(const quint16 address, const quint8 value);

private:
    RAM*        m_ptrRAM = nullptr;

    ROM*        m_ptrBasicROM = nullptr;
    ROM*        m_ptrKernalROM = nullptr;
    ROM*        m_ptrCharROM = nullptr;

    ColorRAM*   m_ptrColorRAM = nullptr;
};
