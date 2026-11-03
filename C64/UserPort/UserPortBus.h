#pragma once

#include <QHash>
#include <QtGlobal>


class UserPortBus
{
public:
    using ParticipantId = quint32;

    enum class Line
    {
        SP,
        CNT
    };

    UserPortBus();
    ~UserPortBus();

    ParticipantId addParticipant();
    bool removeParticipant(ParticipantId participant);

    bool setLineLow(ParticipantId participant, Line line, bool low);
    bool lineHigh(Line line) const;

private:
    static quint8 lineMask(Line line);
    void updateLineStates();

    QHash<ParticipantId, quint8> m_participants;
    ParticipantId m_nextParticipantId = 1;
    quint8 m_linesLow = 0;
};
