#pragma once

#include <QHash>
#include <QtGlobal>


class IECBus
{
public:
    using ParticipantId = quint32;

    enum class Line
    {
        SRQ,
        ATN,
        CLK,
        DATA,
        RESET
    };

    IECBus();
    ~IECBus();

    ParticipantId addParticipant();
    bool removeParticipant(ParticipantId participant);

    bool setLineLow(ParticipantId participant, Line line, bool low);
    bool lineHigh(Line line) const;

    void dumpLineStates() const;

private:
    static quint8 lineMask(Line line);
    void updateLineStates();

    QHash<ParticipantId, quint8> m_participants;
    ParticipantId m_nextParticipantId = 1;

    // Bit gesetzt = mindestens ein Teilnehmer zieht die Leitung auf Low.
    quint8 m_linesLow = 0;
};
