#pragma once

#include <QObject>


class C64MachineTest : public QObject
{
    Q_OBJECT

private slots:
    void testLoadROMSet();
    void testLoadInvalidROMSet();
};
