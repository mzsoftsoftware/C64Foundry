#pragma once

#include <QByteArray>


class KernalROM
{
public:
    explicit KernalROM();
    virtual ~KernalROM();

    // Operations
    quint8 read(const quint16 address) const;

private:
    QByteArray m_data;
};
