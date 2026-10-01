#pragma once

#include <QByteArray>
#include <QtGlobal>
#include <QString>


class ROM
{
public:
    static constexpr quint16 MaximumSize = 8192;

    explicit ROM(const quint16 size);
    virtual ~ROM();

    // Operations
    bool load(const QByteArray& data);
    bool load(const QString& fileName);
    quint8 read(const quint16 address) const;

private:
    quint16 m_size = 0;
    quint8 m_data[MaximumSize];
};
