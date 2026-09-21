#pragma once

#include <QByteArray>


class BasicROM
{
public:
    explicit BasicROM();
    virtual ~BasicROM();

    // Operations
    quint8 read(const quint16 address) const;

private:
    QByteArray m_data;
};
