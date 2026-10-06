#include "KeyboardController.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWindow>

#include "Configuration/ConfigurationManager.h"
#include "Configuration/C64Configuration.h"


KeyboardController::KeyboardController(ConfigurationManager* ptrConfigurationManager, QObject* parent)
    : QObject(parent)
    , m_ptrConfigurationManager(ptrConfigurationManager)
{
    if (m_ptrConfigurationManager->activeConfiguration())
    {
        //setConfiguration(m_ptrConfigurationManager->activeConfiguration()->keyboard);
    }
}
KeyboardController::~KeyboardController()
{
}

/*void KeyboardController::setConfiguration(const C64KeyboardConfiguration& configuration)
{
    m_configuration = configuration;
}*/

void KeyboardController::keyPressed(const int key, const quint32 nativeScanCode, const Qt::KeyboardModifiers modifiers, const bool autoRepeat)
{
    Q_UNUSED(key);
    Q_UNUSED(nativeScanCode);
    Q_UNUSED(modifiers);

    if (autoRepeat)
        return;
/*
    const C64KeyboardMapping* ptrMapping = mapping(nativeScanCode);

    if (ptrMapping == nullptr)
        return;

    if (ptrMapping->mode != C64KeyboardMappingMode::Momentary)
        return;

    if (ptrMapping->keys.size() != 1)
        return;

    emit input(
        {
            InputEventType::KeyPress,
            ptrMapping->keys.first()
        });
*/
}

void KeyboardController::keyReleased(const int key, const quint32 nativeScanCode, const Qt::KeyboardModifiers modifiers, const bool autoRepeat)
{
    Q_UNUSED(key);
    Q_UNUSED(nativeScanCode);
    Q_UNUSED(modifiers);

    if (autoRepeat)
        return;
/*
    const C64KeyboardMapping* ptrMapping = mapping(nativeScanCode);

    if (ptrMapping == nullptr)
        return;

    if (ptrMapping->mode != C64KeyboardMappingMode::Momentary)
        return;

    if (ptrMapping->keys.size() != 1)
        return;

    emit input(
        {
            InputEventType::KeyRelease,
            ptrMapping->keys.first()
        });
*/
}

/*const C64KeyboardMapping* KeyboardController::mapping(const quint32 nativeScanCode) const
{
    for (const C64KeyboardMapping& mapping : m_configuration.mappings)
    {
        if (mapping.nativeScanCode == nativeScanCode)
            return &mapping;
    }

    return nullptr;
}*/

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
