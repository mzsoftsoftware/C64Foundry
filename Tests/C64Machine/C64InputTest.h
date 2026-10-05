#pragma once

#include <QObject>


class C64InputTest : public QObject
{
    Q_OBJECT

private slots:
    void testCIA1Keyboard();
    void testCIA1KeyboardReverse();
    void testCIA1KeyboardShiftLock();
    void testRestore();
    void testRestoreAndCIA2NMI();
    void testCIA1KeyboardBusScan();
};