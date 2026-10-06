#pragma once

#include <QtGlobal>

class HostPlatform
{
public:
    enum class Type
    {
        Unknown,
        LinuxX11,
        LinuxWayland,
        Windows,
        MacOS
    };

    HostPlatform();
    explicit HostPlatform(Type type);

    Type type() const                       { return m_type; }

    QString name() const;
    bool setName(const QString& name);

private:
    Type m_type = Type::Unknown;
};