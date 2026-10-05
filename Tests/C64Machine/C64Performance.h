#pragma once

#include <QObject>


class C64Performance : public QObject
{
    Q_OBJECT

private slots:
    void testPerformance();
};
