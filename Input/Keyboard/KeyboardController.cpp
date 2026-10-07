#include "KeyboardController.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWindow>

#include "Configuration/ConfigurationManager.h"
#include "Input/InputEvent.h"


KeyboardController::KeyboardController(ConfigurationManager* ptrConfigurationManager, QObject* parent)
    : QObject(parent)
    , m_ptrConfigurationManager(ptrConfigurationManager)
{
    connect(m_ptrConfigurationManager, &ConfigurationManager::configurationChanged, this, &KeyboardController::updateConfiguration);

    updateConfiguration();
}
KeyboardController::~KeyboardController()
{
}

void KeyboardController::updateConfiguration()
{
    m_mappings.clear();

    const C64Configuration* ptrConfiguration = m_ptrConfigurationManager->activeConfiguration();
    if(ptrConfiguration == nullptr)
        return;

    const QList<C64KeyboardMapping> mappings = ptrConfiguration->keyboard.mappings(ptrConfiguration->platform.type());
    for (const C64KeyboardMapping& mapping : mappings)
    {
        m_mappings.insert(mapping.nativeScanCode, mapping);
    }
}

const C64KeyboardMapping* KeyboardController::mapping(const quint32 nativeScanCode) const
{
    const auto iterator = m_mappings.constFind(nativeScanCode);
    if (iterator == m_mappings.constEnd())
        return nullptr;
    return &iterator.value();
}

void KeyboardController::keyPressed(const int key, const quint32 nativeScanCode, const Qt::KeyboardModifiers modifiers, const bool autoRepeat)
{
    Q_UNUSED(key);
    Q_UNUSED(modifiers);

    if (autoRepeat)
        return;

    const C64KeyboardMapping* ptrMapping = mapping(nativeScanCode);
    if (ptrMapping == nullptr)
        return;

    if (ptrMapping->mode != C64KeyboardMappingMode::Momentary)
        return;

    for (const C64Key key : ptrMapping->keys)
        pressKey(key);
}

void KeyboardController::keyReleased(const int key, const quint32 nativeScanCode, const Qt::KeyboardModifiers modifiers, const bool autoRepeat)
{
    Q_UNUSED(key);
    Q_UNUSED(modifiers);

    if (autoRepeat)
        return;

    const C64KeyboardMapping* ptrMapping = mapping(nativeScanCode);
    if (ptrMapping == nullptr)
        return;

    if (ptrMapping->mode != C64KeyboardMappingMode::Momentary)
        return;

    for (const C64Key key : ptrMapping->keys)
        releaseKey(key);
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
            qDebug()
            << "Key press:"
            << "key =" << ptrKeyEvent->key()
            << "scan =" << ptrKeyEvent->nativeScanCode()
            << "nativeVirtualKey =" << ptrKeyEvent->nativeVirtualKey()
            << "nativeModifiers =" << ptrKeyEvent->nativeModifiers();

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

void KeyboardController::pressKey(const C64Key key)
{
    int& count = m_pressedKeys[key];
    ++count;
    if (count != 1)
        return;
    emit input(
        {
            InputEventType::KeyPress,
            key
        });
}

void KeyboardController::releaseKey(const C64Key key)
{
    if (!m_pressedKeys.contains(key))
        return;

    --m_pressedKeys[key];
    if (m_pressedKeys[key] != 0)
        return;

    m_pressedKeys.remove(key);
    emit input(
        {
            InputEventType::KeyRelease,
            key
        });
}
