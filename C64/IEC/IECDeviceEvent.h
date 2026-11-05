#pragma once

#include <QMetaType>
#include <QQueue>
#include <QString>
#include <QtGlobal>


struct IECDriveAccessEvent
{
    enum class Operation : quint8
    {
        Read,
        Write
    };

    quint8 deviceNumber = 0;
    Operation operation = Operation::Read;
    quint8 track = 0;
    quint8 sector = 0;
    bool success = false;
    QString errorMessage;
};


Q_DECLARE_METATYPE(IECDriveAccessEvent)
Q_DECLARE_METATYPE(QQueue<IECDriveAccessEvent>)
