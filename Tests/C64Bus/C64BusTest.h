#pragma once

#include <QObject>


class C64BusTest : public QObject
{
    Q_OBJECT

public:
    explicit C64BusTest();
    virtual ~C64BusTest();

private slots:
    void testInitialCpuPortLines();
    void testSetCpuPortLines();
};