#pragma once

#include <QByteArray>
#include <QtGlobal>


struct IECProtocolEvent
{
    enum class Type : quint8
    {
        DeviceSelected,
        SecondaryAddressReceived,
        ByteReceived,
        EndOfTransmission,
        DeviceDeselected
    };

    enum class Mode : quint8
    {
        None,
        Listen,
        Talk
    };

    Type type = Type::DeviceSelected;

    quint8 deviceNumber = 0;
    Mode mode = Mode::None;

    quint8 secondaryAddress = 0;
    quint8 value = 0;

    bool eoi = false;
};


struct IECProtocolResponse
{
    enum class Type : quint8
    {
        None,
        SendData
    };

    Type type = Type::None;
    QByteArray data;
};
