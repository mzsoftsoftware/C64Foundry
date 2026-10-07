#pragma once

#include <QList>
#include <QtGlobal>

#include "C64/Input/C64Keyboard.h"
#include "HostPlatform.h"


enum class C64KeyboardMappingMode
{
    Momentary,
    Toggle
};
struct C64KeyboardMapping
{
    quint32 nativeScanCode;
    QList<C64Key> keys;
    C64KeyboardMappingMode mode = C64KeyboardMappingMode::Momentary;
};


class C64KeyboardConfiguration
{
public:
    C64KeyboardConfiguration() = default;

    QList<C64KeyboardMapping> mappings(HostPlatform::Type platform) const;

    const QList<C64KeyboardMapping>& overrides() const          { return m_overrides; }

    void setOverride(const C64KeyboardMapping& mapping);
    void removeOverride(quint32 nativeScanCode);
    void clearOverrides();

    static QString keyName(C64Key key);
    static bool keyFromName(const QString& name, C64Key& key);

    static QString mappingModeName(C64KeyboardMappingMode mode);
    static bool mappingModeFromName(const QString& name, C64KeyboardMappingMode& mode);

private:
    QList<C64KeyboardMapping> m_overrides;
};
