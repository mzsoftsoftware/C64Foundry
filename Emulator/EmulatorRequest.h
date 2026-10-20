#pragma once

#include <QByteArray>
#include <QtGlobal>

#include "EmulatorTypes.h"
#include "C64/C64ROMSet.h"
#include "Input/InputEvent.h"


class EmulatorRequest
{
public:
    using RequestId = quint64;

    enum class Type
    {
        LoadROMSet,
        LoadProgram,
        Input,
        PowerOn,
        PowerOff,
        Pause,
        SoftReset,
        HardReset,
        SetSpeed,
        Shutdown
    };

    EmulatorRequest(Type type, RequestId id)
        : m_type(type)
        , m_id(id)
    {
    }

    virtual ~EmulatorRequest() = default;

    EmulatorRequest(const EmulatorRequest&) = delete;
    EmulatorRequest& operator=(const EmulatorRequest&) = delete;

    Type type() const { return m_type; }
    RequestId id() const { return m_id; }

private:
    Type m_type;
    RequestId m_id;
};


class EmulatorRequestLoadROMSet : public EmulatorRequest
{
public:
    EmulatorRequestLoadROMSet(RequestId id, const C64ROMSet& romSet)
        : EmulatorRequest(Type::LoadROMSet, id)
        , m_romSet(romSet)
    {
    }

    const C64ROMSet& romSet() const { return m_romSet; }

private:
    C64ROMSet m_romSet;
};


class EmulatorRequestLoadProgram : public EmulatorRequest
{
public:
    EmulatorRequestLoadProgram(RequestId id, quint16 address, const QByteArray& data)
        : EmulatorRequest(Type::LoadProgram, id)
        , m_address(address)
        , m_data(data)
    {
    }

    quint16 address() const { return m_address; }
    const QByteArray& data() const { return m_data; }

private:
    quint16 m_address;
    QByteArray m_data;
};


class EmulatorRequestInput : public EmulatorRequest
{
public:
    EmulatorRequestInput(RequestId id, const InputEvent& event)
        : EmulatorRequest(Type::Input, id)
        , m_event(event)
    {
    }

    const InputEvent& event() const { return m_event; }

private:
    InputEvent m_event;
};


class EmulatorRequestPowerOn : public EmulatorRequest
{
public:
    explicit EmulatorRequestPowerOn(RequestId id)
        : EmulatorRequest(Type::PowerOn, id)
    {
    }
};


class EmulatorRequestPowerOff : public EmulatorRequest
{
public:
    explicit EmulatorRequestPowerOff(RequestId id)
        : EmulatorRequest(Type::PowerOff, id)
    {
    }
};


class EmulatorRequestPause : public EmulatorRequest
{
public:
    EmulatorRequestPause(RequestId id, bool paused)
        : EmulatorRequest(Type::Pause, id)
        , m_paused(paused)
    {
    }

    bool paused() const { return m_paused; }

private:
    bool m_paused;
};


class EmulatorRequestSoftReset : public EmulatorRequest
{
public:
    explicit EmulatorRequestSoftReset(RequestId id)
        : EmulatorRequest(Type::SoftReset, id)
    {
    }
};


class EmulatorRequestHardReset : public EmulatorRequest
{
public:
    explicit EmulatorRequestHardReset(RequestId id)
        : EmulatorRequest(Type::HardReset, id)
    {
    }
};


class EmulatorRequestSetSpeed : public EmulatorRequest
{
public:
    EmulatorRequestSetSpeed(RequestId id, Emulator::Speed speed)
        : EmulatorRequest(Type::SetSpeed, id)
        , m_speed(speed)
    {
    }

    Emulator::Speed speed() const { return m_speed; }

private:
    Emulator::Speed m_speed;
};


class EmulatorRequestShutdown : public EmulatorRequest
{
public:
    explicit EmulatorRequestShutdown(RequestId id)
        : EmulatorRequest(Type::Shutdown, id)
    {
    }
};
