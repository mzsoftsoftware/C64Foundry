#pragma once

#include <QByteArray>
#include <QtGlobal>
#include <QString>


class ROM
{
public:
    explicit ROM(const qsizetype size);
    virtual ~ROM();

    // Operations
    bool load(const QByteArray& data);
    bool load(const QString& fileName);
    quint8 read(const quint16 address) const;

private:
    qsizetype m_size = 0;
    QByteArray m_data;
};
