#include "HostPlatform.h"

#include <QGuiApplication>


HostPlatform::HostPlatform()
{
    const QString platformName = QGuiApplication::platformName();

    if (platformName == QStringLiteral("xcb"))
        m_type = Type::LinuxX11;
    else if (platformName.startsWith(QStringLiteral("wayland")))
        m_type = Type::LinuxWayland;
    else if (platformName == QStringLiteral("windows"))
        m_type = Type::Windows;
    else if (platformName == QStringLiteral("cocoa"))
        m_type = Type::MacOS;
}
HostPlatform::HostPlatform(Type type)
    : m_type(type)
{
}

QString HostPlatform::name() const
{
    switch (m_type)
    {
    case Type::LinuxX11:
        return QStringLiteral("LinuxX11");
    case Type::LinuxWayland:
        return QStringLiteral("LinuxWayland");
    case Type::Windows:
        return QStringLiteral("Windows");
    case Type::MacOS:
        return QStringLiteral("MacOS");
    case Type::Unknown:
        return QStringLiteral("Unknown");
    }
    return QStringLiteral("Unknown");
}

bool HostPlatform::setName(const QString& name)
{
    if (name == QStringLiteral("LinuxX11"))
        m_type = Type::LinuxX11;
    else if (name == QStringLiteral("LinuxWayland"))
        m_type = Type::LinuxWayland;
    else if (name == QStringLiteral("Windows"))
        m_type = Type::Windows;
    else if (name == QStringLiteral("MacOS"))
        m_type = Type::MacOS;
    else
    {
        m_type = Type::Unknown;
        return false;
    }
    return true;
}
