#include "UserPortBus.h"


UserPortBus::UserPortBus()
{
}

UserPortBus::~UserPortBus()
{
}

UserPortBus::ParticipantId UserPortBus::addParticipant()
{
    const ParticipantId participant = m_nextParticipantId++;

    m_participants.insert(participant, 0x00);

    return participant;
}

bool UserPortBus::removeParticipant(const ParticipantId participant)
{
    if (m_participants.remove(participant) == 0)
        return false;

    updateLineStates();

    return true;
}

bool UserPortBus::setLineLow(const ParticipantId participant, const Line line, const bool low)
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

bool UserPortBus::lineHigh(const Line line) const
{
    return (m_linesLow & lineMask(line)) == 0;
}

void UserPortBus::updateLineStates()
{
    quint8 linesLow = 0;

    for (QHash<ParticipantId, quint8>::const_iterator it = m_participants.cbegin(); it != m_participants.cend(); ++it)
        linesLow |= it.value();

    m_linesLow = linesLow;
}

quint8 UserPortBus::lineMask(const Line line)
{
    switch (line)
    {
    case Line::SP:
        return 0x01;

    case Line::CNT:
        return 0x02;
    }

    return 0x00;
}
