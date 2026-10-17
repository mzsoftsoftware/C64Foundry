#pragma once

#include <QtGlobal>


class ColorRAM
{
public:
    explicit ColorRAM();
    virtual ~ColorRAM();

    void initialize();

    quint8 read(const quint16 address) const;
    void write(const quint16 address, const quint8 value);

private:
    quint8 m_data[1024];
};
