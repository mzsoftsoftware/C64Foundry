#include "C64KeyboardDefaults.h"


bool C64KeyboardDefaults::isSupported(const HostPlatform::Type platform)
{
    return platform == HostPlatform::Type::LinuxWayland;
}

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
                { 105,  { C64Key::Pound } },        // STRG Rechts
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
                { 66, { C64Key::ShiftLock }, C64KeyboardMappingMode::Toggle },
                { 38, { C64Key::KeyA } },
                { 39, { C64Key::KeyS } },
                { 40, { C64Key::KeyD } },
                { 41, { C64Key::KeyF } },
                { 42, { C64Key::KeyG } },
                { 43, { C64Key::KeyH } },
                { 44, { C64Key::KeyJ } },
                { 45, { C64Key::KeyK } },
                { 46, { C64Key::KeyL } },
                { 47, { C64Key::Colon } },       // Ö
                { 48, { C64Key::Semicolon } },   // Ä
                { 51, { C64Key::Equals } },      // #
                { 36, { C64Key::Return } },

                { 37, { C64Key::Commodore } },  // STRG Links
                { 50, { C64Key::LeftShift } },
                { 52, { C64Key::KeyZ } },
                { 53, { C64Key::KeyX } },
                { 54, { C64Key::KeyC } },
                { 55, { C64Key::KeyV } },
                { 56, { C64Key::KeyB } },
                { 57, { C64Key::KeyN } },
                { 58, { C64Key::KeyM } },
                { 59, { C64Key::Comma } },
                { 60, { C64Key::Period } },
                { 61, { C64Key::Slash } },
                { 62, { C64Key::RightShift } },

                { 65, { C64Key::Space } },
                { 111, { C64Key::LeftShift, C64Key::CursorUpDown } },       // Up
                { 116, { C64Key::CursorUpDown } },                           // Down
                { 113, { C64Key::LeftShift, C64Key::CursorLeftRight } },    // Left
                { 114, { C64Key::CursorLeftRight } },                        // Right
                { 67, { C64Key::F1F2 } },                       // F1
                { 68, { C64Key::LeftShift, C64Key::F1F2 } },   // F2
                { 69, { C64Key::F3F4 } },                       // F3
                { 70, { C64Key::LeftShift, C64Key::F3F4 } },   // F4
                { 71, { C64Key::F5F6 } },                       // F5
                { 72, { C64Key::LeftShift, C64Key::F5F6 } },   // F6
                { 73, { C64Key::F7F8 } },                       // F7
                { 74, { C64Key::LeftShift, C64Key::F7F8 } },   // F8

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