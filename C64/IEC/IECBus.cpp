#include "IECBus.h"

#include <QDebug>


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
    if (m_participants.remove(participant) == 0)
        return false;

    updateLineStates();

    return true;
}


bool IECBus::setLineLow(const ParticipantId participant, const Line line, const bool low)
{
    QHash<ParticipantId, quint8>::iterator it = m_participants.find(participant);

    if (it == m_participants.end())
        return false;

    const quint8 mask = lineMask(line);
    const quint8 previousState = it.value();

    if (low)
        it.value() |= mask;
    else
        it.value() &= static_cast<quint8>(~mask);

    if (it.value() == previousState)
        return true;

    updateLineStates();

    return true;
}

bool IECBus::lineHigh(const Line line) const
{
    return (m_linesLow & lineMask(line)) == 0;
}

void IECBus::updateLineStates()
{
    quint8 linesLow = 0;

    for (QHash<ParticipantId, quint8>::const_iterator it = m_participants.cbegin(); it != m_participants.cend(); ++it)
        linesLow |= it.value();

    m_linesLow = linesLow;
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
