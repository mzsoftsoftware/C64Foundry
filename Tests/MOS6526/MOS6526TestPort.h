#pragma once

#include <QObject>


class MOS6526TestPort : public QObject
{
    Q_OBJECT

public:
    explicit MOS6526TestPort();
    virtual ~MOS6526TestPort();

private slots:
    void testPortARegisterWrite();
};
