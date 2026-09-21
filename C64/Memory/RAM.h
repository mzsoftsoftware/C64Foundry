#pragma once

#include <QByteArray>


class RAM
{
public:
    explicit RAM();
    virtual ~RAM();

    quint8 read(const quint16 address) const;
    void write(const quint16 address, const quint8 value);

private:
    QByteArray m_data;
};
