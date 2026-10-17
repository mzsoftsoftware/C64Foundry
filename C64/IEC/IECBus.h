#pragma once

#include <QHash>
#include <QtGlobal>


class IECBus
{
public:
    using ParticipantId = quint32;

    enum class Line
    {
        ATN,
        CLK,
        DATA
    };

    IECBus();
    ~IECBus();

    ParticipantId addParticipant();
    bool removeParticipant(ParticipantId participant);

    bool setLineLow(ParticipantId participant, Line line, bool low);
    bool lineHigh(Line line) const;

private:
    static quint8 lineMask(Line line);

    QHash<ParticipantId, quint8> m_participants;
    ParticipantId m_nextParticipantId = 1;
};
