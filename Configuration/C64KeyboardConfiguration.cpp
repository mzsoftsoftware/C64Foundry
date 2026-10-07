#include "C64KeyboardConfiguration.h"

#include "C64KeyboardDefaults.h"


QList<C64KeyboardMapping> C64KeyboardConfiguration::mappings(const HostPlatform::Type platform) const
{
    QList<C64KeyboardMapping> mappings = C64KeyboardDefaults::mappings(platform);
    for (const C64KeyboardMapping& overrideMapping : m_overrides)
    {
        int mappingIndex = -1;
        for (int index = 0; index < mappings.size(); ++index)
        {
            if (mappings[index].nativeScanCode == overrideMapping.nativeScanCode)
            {
                mappingIndex = index;
                break;
            }
        }

        if (overrideMapping.keys.isEmpty())
        {
            if (mappingIndex >= 0)
                mappings.removeAt(mappingIndex);

            continue;
        }

        if (mappingIndex >= 0)
            mappings[mappingIndex] = overrideMapping;
        else
            mappings.append(overrideMapping);
    }

    return mappings;
}

void C64KeyboardConfiguration::setOverride(const C64KeyboardMapping& mapping)
{
    for (int index = 0; index < m_overrides.size(); ++index)
    {
        if (m_overrides[index].nativeScanCode == mapping.nativeScanCode)
        {
            m_overrides[index] = mapping;
            return;
        }
    }

    m_overrides.append(mapping);
}

void C64KeyboardConfiguration::removeOverride(const quint32 nativeScanCode)
{
    for (int index = 0; index < m_overrides.size(); ++index)
    {
        if (m_overrides[index].nativeScanCode == nativeScanCode)
        {
            m_overrides.removeAt(index);
            return;
        }
    }
}

void C64KeyboardConfiguration::clearOverrides()
{
    m_overrides.clear();
}


QString C64KeyboardConfiguration::keyName(const C64Key key)
{
    switch (key)
    {
    case C64Key::InsertDelete:       return QStringLiteral("InsertDelete");
    case C64Key::Return:             return QStringLiteral("Return");
    case C64Key::CursorLeftRight:    return QStringLiteral("CursorLeftRight");
    case C64Key::F7F8:               return QStringLiteral("F7F8");
    case C64Key::F1F2:               return QStringLiteral("F1F2");
    case C64Key::F3F4:               return QStringLiteral("F3F4");
    case C64Key::F5F6:               return QStringLiteral("F5F6");
    case C64Key::CursorUpDown:       return QStringLiteral("CursorUpDown");

    case C64Key::Key3:               return QStringLiteral("Key3");
    case C64Key::KeyW:               return QStringLiteral("KeyW");
    case C64Key::KeyA:               return QStringLiteral("KeyA");
    case C64Key::Key4:               return QStringLiteral("Key4");
    case C64Key::KeyZ:               return QStringLiteral("KeyZ");
    case C64Key::KeyS:               return QStringLiteral("KeyS");
    case C64Key::KeyE:               return QStringLiteral("KeyE");
    case C64Key::LeftShift:          return QStringLiteral("LeftShift");

    case C64Key::Key5:               return QStringLiteral("Key5");
    case C64Key::KeyR:               return QStringLiteral("KeyR");
    case C64Key::KeyD:               return QStringLiteral("KeyD");
    case C64Key::Key6:               return QStringLiteral("Key6");
    case C64Key::KeyC:               return QStringLiteral("KeyC");
    case C64Key::KeyF:               return QStringLiteral("KeyF");
    case C64Key::KeyT:               return QStringLiteral("KeyT");
    case C64Key::KeyX:               return QStringLiteral("KeyX");

    case C64Key::Key7:               return QStringLiteral("Key7");
    case C64Key::KeyY:               return QStringLiteral("KeyY");
    case C64Key::KeyG:               return QStringLiteral("KeyG");
    case C64Key::Key8:               return QStringLiteral("Key8");
    case C64Key::KeyB:               return QStringLiteral("KeyB");
    case C64Key::KeyH:               return QStringLiteral("KeyH");
    case C64Key::KeyU:               return QStringLiteral("KeyU");
    case C64Key::KeyV:               return QStringLiteral("KeyV");

    case C64Key::Key9:               return QStringLiteral("Key9");
    case C64Key::KeyI:               return QStringLiteral("KeyI");
    case C64Key::KeyJ:               return QStringLiteral("KeyJ");
    case C64Key::Key0:               return QStringLiteral("Key0");
    case C64Key::KeyM:               return QStringLiteral("KeyM");
    case C64Key::KeyK:               return QStringLiteral("KeyK");
    case C64Key::KeyO:               return QStringLiteral("KeyO");
    case C64Key::KeyN:               return QStringLiteral("KeyN");

    case C64Key::Plus:               return QStringLiteral("Plus");
    case C64Key::KeyP:               return QStringLiteral("KeyP");
    case C64Key::KeyL:               return QStringLiteral("KeyL");
    case C64Key::Minus:              return QStringLiteral("Minus");
    case C64Key::Period:             return QStringLiteral("Period");
    case C64Key::Colon:              return QStringLiteral("Colon");
    case C64Key::At:                 return QStringLiteral("At");
    case C64Key::Comma:              return QStringLiteral("Comma");

    case C64Key::Pound:              return QStringLiteral("Pound");
    case C64Key::Asterisk:           return QStringLiteral("Asterisk");
    case C64Key::Semicolon:          return QStringLiteral("Semicolon");
    case C64Key::HomeClear:          return QStringLiteral("HomeClear");
    case C64Key::RightShift:         return QStringLiteral("RightShift");
    case C64Key::Equals:             return QStringLiteral("Equals");
    case C64Key::ArrowUp:            return QStringLiteral("ArrowUp");
    case C64Key::Slash:              return QStringLiteral("Slash");

    case C64Key::Key1:               return QStringLiteral("Key1");
    case C64Key::ArrowLeft:          return QStringLiteral("ArrowLeft");
    case C64Key::Control:            return QStringLiteral("Control");
    case C64Key::Key2:               return QStringLiteral("Key2");
    case C64Key::Space:              return QStringLiteral("Space");
    case C64Key::Commodore:          return QStringLiteral("Commodore");
    case C64Key::KeyQ:               return QStringLiteral("KeyQ");
    case C64Key::RunStop:            return QStringLiteral("RunStop");

    case C64Key::ShiftLock:          return QStringLiteral("ShiftLock");
    case C64Key::Restore:            return QStringLiteral("Restore");
    }
    return QString();
}
bool C64KeyboardConfiguration::keyFromName(const QString& name, C64Key& key)
{
    for (int value = static_cast<int>(C64Key::InsertDelete); value <= static_cast<int>(C64Key::Restore); ++value)
    {
        const C64Key currentKey = static_cast<C64Key>(value);
        if (keyName(currentKey) == name)
        {
            key = currentKey;
            return true;
        }
    }
    return false;
}

QString C64KeyboardConfiguration::mappingModeName(const C64KeyboardMappingMode mode)
{
    switch (mode)
    {
    case C64KeyboardMappingMode::Momentary:
        return QStringLiteral("Momentary");

    case C64KeyboardMappingMode::Toggle:
        return QStringLiteral("Toggle");
    }
    return QString();
}
bool C64KeyboardConfiguration::mappingModeFromName(const QString& name, C64KeyboardMappingMode& mode)
{
    if (name == QStringLiteral("Momentary"))
    {
        mode = C64KeyboardMappingMode::Momentary;
        return true;
    }
    if (name == QStringLiteral("Toggle"))
    {
        mode = C64KeyboardMappingMode::Toggle;
        return true;
    }
    return false;
}