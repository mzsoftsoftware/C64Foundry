#include "ROM.h"

#include <QFile>


ROM::ROM(const quint16 size)
    : m_size(size)
{
    std::memset(m_data, 0, sizeof(m_data));
}
ROM::~ROM()
{
}

bool ROM::load(const QByteArray& data)
{
    if (data.size() != m_size)
        return false;

    std::memcpy(m_data, data.constData(), m_size);

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
    return m_data[address];
}
