#pragma once

#include <QByteArray>


class ColorRAM
{
public:
    explicit ColorRAM();
    virtual ~ColorRAM();

    quint8 read(const quint16 address) const;
    void write(const quint16 address, const quint8 value);

private:
    QByteArray m_data;
};
