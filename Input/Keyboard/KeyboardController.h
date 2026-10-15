#pragma once

#include <QObject>
#include <QHash>
#include <QSet>

class InputEvent;
class ConfigurationManager;

#include "Configuration/C64Configuration.h"


class KeyboardController : public QObject
{
    Q_OBJECT
public:
    explicit KeyboardController(ConfigurationManager* ptrConfigurationManager, QObject* parent);
    virtual ~KeyboardController();

public slots:
    void keyPressed(int key, const quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);
    void keyReleased(int key, const quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);
    void virtualKeyPressed(C64Key key);
    void virtualKeyReleased(C64Key key);

signals:
    void input(const InputEvent& event);
    void keyStateChanged(C64Key key, bool bPressed);
    void keyboardStateReset();

protected:
    bool eventFilter(QObject* ptrObject, QEvent* ptrEvent) override;

private slots:
    void updateConfiguration();

private:
    const C64KeyboardMapping* mapping(quint32 nativeScanCode) const;

    void pressKey(C64Key key);
    void releaseKey(C64Key key);
    void pressPhysicalKey(C64Key key);
    void releasePhysicalKey(C64Key key);
    void resetKeyboardState();

private:
    ConfigurationManager* m_ptrConfigurationManager = nullptr;

    QHash<quint32, C64KeyboardMapping> m_mappings;
    QHash<C64Key, int> m_pressedKeys;
    QHash<C64Key, int> m_physicalPressedKeys;
    QSet<C64Key> m_virtualPressedKeys;
    QSet<quint32> m_activeToggles;
};
