#pragma once

#include <QByteArray>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>


struct IECDeviceEvent
{
    virtual ~IECDeviceEvent() = default;

    IECDeviceConfiguration::DeviceId deviceId = 0;
};

struct IECPrinterOutputEvent : public IECDeviceEvent
{
    enum class Format : quint8
    {
        Text,
        Graphics,
        Page
    };

    Format format = Format::Text;
    QByteArray data;
};

struct IECDriveAccessEvent : public IECDeviceEvent
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


using IECDeviceEventPtr = QSharedPointer<IECDeviceEvent>;
using IECDeviceEventList = QList<IECDeviceEventPtr>;
