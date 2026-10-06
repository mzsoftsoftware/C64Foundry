#include "C64KeyboardDefaults.h"


QList<C64KeyboardMapping> C64KeyboardDefaults::mappings(const HostPlatform::Type platform)
{
    QList<C64KeyboardMapping> mappings;

    switch (platform)
    {
    case HostPlatform::Type::LinuxWayland:
        mappings =
            {
                { 49, { C64Key::ArrowLeft } },
                { 10, { C64Key::Key1 } },
                { 11, { C64Key::Key2 } },
                { 12, { C64Key::Key3 } },
                { 13, { C64Key::Key4 } },
                { 14, { C64Key::Key5 } },
                { 15, { C64Key::Key6 } },
                { 16, { C64Key::Key7 } },
                { 17, { C64Key::Key8 } },
                { 18, { C64Key::Key9 } },
                { 19, { C64Key::Key0 } },
                { 20,  { C64Key::Plus } },
                { 21,  { C64Key::Minus } },
                { 51,  { C64Key::Pound } },
                { 22,  { C64Key::InsertDelete } },
                { 110, { C64Key::HomeClear } },
                { 119, { C64Key::LeftShift, C64Key::HomeClear } },
                { 118, { C64Key::LeftShift, C64Key::InsertDelete } },

                { 23, { C64Key::Control } },
                { 24, { C64Key::KeyQ } },
                { 25, { C64Key::KeyW } },
                { 26, { C64Key::KeyE } },
                { 27, { C64Key::KeyR } },
                { 28, { C64Key::KeyT } },
                { 29, { C64Key::KeyY } },
                { 30, { C64Key::KeyU } },
                { 31, { C64Key::KeyI } },
                { 32, { C64Key::KeyO } },
                { 33, { C64Key::KeyP } },
                { 34, { C64Key::At } },
                { 35, { C64Key::Asterisk } },
                { 112, { C64Key::ArrowUp } },
                { 117, { C64Key::Restore } },

                { 9, { C64Key::RunStop } },
                { 66, { C64Key::ShiftLock }, C64KeyboardMappingMode::Toggle }
            };
        break;

    case HostPlatform::Type::LinuxX11:
    case HostPlatform::Type::Windows:
    case HostPlatform::Type::MacOS:
    case HostPlatform::Type::Unknown:
        break;
    }

    return mappings;
}