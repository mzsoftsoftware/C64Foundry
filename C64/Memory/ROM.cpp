#include "ROM.h"

#include <QFile>


ROM::ROM(const qsizetype size)
    : m_size(size)
    , m_data(size, char(0x00))
{
}
ROM::~ROM()
{
}

bool ROM::load(const QByteArray& data)
{
    if (data.size() != m_size)
        return false;

    m_data = data;

    return true;
}
bool ROM::load(const QString& fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    const QByteArray data = file.readAll();

    return load(data);
}

quint8 ROM::read(const quint16 address) const
{
    return static_cast<quint8>(m_data.at(address));
}
