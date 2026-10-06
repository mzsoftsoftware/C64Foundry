#include "KeyboardController.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWindow>


KeyboardController::KeyboardController(QObject* parent)
    : QObject{parent}
{
}
KeyboardController::~KeyboardController()
{
}

void KeyboardController::keyPressed(const int key, const quint32 nativeScanCode, const Qt::KeyboardModifiers modifiers, const bool autoRepeat)
{
    Q_UNUSED(modifiers);

    if (autoRepeat)
        return;

    C64Key c64Key;

    if (!mapKey(key, nativeScanCode, c64Key))
        return;

    emit input(
        {
            InputEventType::KeyPress,
            c64Key
        });
}

void KeyboardController::keyReleased(const int key, const quint32 nativeScanCode, const Qt::KeyboardModifiers modifiers, const bool autoRepeat)
{
    Q_UNUSED(modifiers);

    if (autoRepeat)
        return;

    C64Key c64Key;

    if (!mapKey(key, nativeScanCode, c64Key))
        return;

    emit input(
        {
            InputEventType::KeyRelease,
            c64Key
        });
}

bool KeyboardController::mapKey(int key, quint32 nativeScanCode, C64Key& c64Key) const
{
    Q_UNUSED(nativeScanCode);

    switch (key)
    {
    case Qt::Key_A: c64Key = C64Key::KeyA; return true;
    case Qt::Key_B: c64Key = C64Key::KeyB; return true;
    case Qt::Key_C: c64Key = C64Key::KeyC; return true;
    case Qt::Key_D: c64Key = C64Key::KeyD; return true;
    case Qt::Key_E: c64Key = C64Key::KeyE; return true;
    case Qt::Key_F: c64Key = C64Key::KeyF; return true;
    case Qt::Key_G: c64Key = C64Key::KeyG; return true;
    case Qt::Key_H: c64Key = C64Key::KeyH; return true;
    case Qt::Key_I: c64Key = C64Key::KeyI; return true;
    case Qt::Key_J: c64Key = C64Key::KeyJ; return true;
    case Qt::Key_K: c64Key = C64Key::KeyK; return true;
    case Qt::Key_L: c64Key = C64Key::KeyL; return true;
    case Qt::Key_M: c64Key = C64Key::KeyM; return true;
    case Qt::Key_N: c64Key = C64Key::KeyN; return true;
    case Qt::Key_O: c64Key = C64Key::KeyO; return true;
    case Qt::Key_P: c64Key = C64Key::KeyP; return true;
    case Qt::Key_Q: c64Key = C64Key::KeyQ; return true;
    case Qt::Key_R: c64Key = C64Key::KeyR; return true;
    case Qt::Key_S: c64Key = C64Key::KeyS; return true;
    case Qt::Key_T: c64Key = C64Key::KeyT; return true;
    case Qt::Key_U: c64Key = C64Key::KeyU; return true;
    case Qt::Key_V: c64Key = C64Key::KeyV; return true;
    case Qt::Key_W: c64Key = C64Key::KeyW; return true;
    case Qt::Key_X: c64Key = C64Key::KeyX; return true;
    case Qt::Key_Y: c64Key = C64Key::KeyY; return true;
    case Qt::Key_Z: c64Key = C64Key::KeyZ; return true;

    case Qt::Key_0: c64Key = C64Key::Key0; return true;
    case Qt::Key_1: c64Key = C64Key::Key1; return true;
    case Qt::Key_2: c64Key = C64Key::Key2; return true;
    case Qt::Key_3: c64Key = C64Key::Key3; return true;
    case Qt::Key_4: c64Key = C64Key::Key4; return true;
    case Qt::Key_5: c64Key = C64Key::Key5; return true;
    case Qt::Key_6: c64Key = C64Key::Key6; return true;
    case Qt::Key_7: c64Key = C64Key::Key7; return true;
    case Qt::Key_8: c64Key = C64Key::Key8; return true;
    case Qt::Key_9: c64Key = C64Key::Key9; return true;

    case Qt::Key_Space:  c64Key = C64Key::Space;  return true;
    case Qt::Key_Return:
    case Qt::Key_Enter:  c64Key = C64Key::Return; return true;

    default:
        return false;
    }
}

bool KeyboardController::eventFilter(QObject* ptrObject, QEvent* ptrEvent)
{
    if (ptrEvent->type() == QEvent::KeyPress ||
        ptrEvent->type() == QEvent::KeyRelease)
    {
        if (qobject_cast<QWindow*>(ptrObject) == nullptr)
            return false;

        QKeyEvent* ptrKeyEvent =
            static_cast<QKeyEvent*>(ptrEvent);

        if (ptrEvent->type() == QEvent::KeyPress)
        {
            keyPressed(
                ptrKeyEvent->key(),
                ptrKeyEvent->nativeScanCode(),
                ptrKeyEvent->modifiers(),
                ptrKeyEvent->isAutoRepeat());
        }
        else
        {
            keyReleased(
                ptrKeyEvent->key(),
                ptrKeyEvent->nativeScanCode(),
                ptrKeyEvent->modifiers(),
                ptrKeyEvent->isAutoRepeat());
        }

        return true;
    }

    return QObject::eventFilter(ptrObject, ptrEvent);
}
