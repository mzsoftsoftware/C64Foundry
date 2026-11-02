#pragma once

#include <QObject>


class MOS6526TestTOD : public QObject
{
    Q_OBJECT

public:
    explicit MOS6526TestTOD();
    virtual ~MOS6526TestTOD();

private slots:
    void testTODInitialState();
    void testTODTenthsDivider();
    void testTODSecondsAndMinutesRollover();
    void testTODHoursRollover();
    void testTODReadLatch();
    void testTODClockStopAndRestart();
    void testTODAlarmInterrupt();
    void testTODSixtyHertzDivider();
    void testTODRegisterMirroring();
};
