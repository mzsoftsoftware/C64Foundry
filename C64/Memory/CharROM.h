#pragma once

#include <QByteArray>


class CharROM
{
public:
    explicit CharROM();
    virtual ~CharROM();

    // Operations
    quint8 read(const quint16 address) const;

private:
    QByteArray m_data;
};
