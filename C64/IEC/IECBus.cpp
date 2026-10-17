#include "IECBus.h"


IECBus::IECBus()
{
}

IECBus::~IECBus()
{
}

IECBus::ParticipantId IECBus::addParticipant()
{
    const ParticipantId participant = m_nextParticipantId++;

    m_participants.insert(participant, 0x00);

    return participant;
}

bool IECBus::removeParticipant(const ParticipantId participant)
{
    return m_participants.remove(participant) != 0;
}

bool IECBus::setLineLow(const ParticipantId participant, const Line line, const bool low)
{
    QHash<ParticipantId, quint8>::iterator it = m_participants.find(participant);

    if (it == m_participants.end())
        return false;

    const quint8 mask = lineMask(line);

    if (low)
        it.value() |= mask;
    else
        it.value() &= static_cast<quint8>(~mask);

    return true;
}

bool IECBus::lineHigh(const Line line) const
{
    const quint8 mask = lineMask(line);

    for (QHash<ParticipantId, quint8>::const_iterator it = m_participants.cbegin(); it != m_participants.cend(); ++it)
    {
        if ((it.value() & mask) != 0)
            return false;
    }

    return true;
}

quint8 IECBus::lineMask(const Line line)
{
    switch (line)
    {
    case Line::SRQ:
        return 0x01;

    case Line::ATN:
        return 0x02;

    case Line::CLK:
        return 0x04;

    case Line::DATA:
        return 0x08;

    case Line::RESET:
        return 0x10;
    }

    return 0x00;
}
