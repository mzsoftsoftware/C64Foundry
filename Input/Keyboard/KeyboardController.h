#pragma once

#include <QObject>

//#include "Input/InputEvent.h"
class InputEvent;
class ConfigurationManager;
class C64KeyboardConfiguration;


class KeyboardController : public QObject
{
    Q_OBJECT
public:
    explicit KeyboardController(ConfigurationManager* ptrConfigurationManager, QObject* parent);
    virtual ~KeyboardController();

public slots:
    void keyPressed(int key, const quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);
    void keyReleased(int key, const quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);

signals:
    void input(const InputEvent& event);

protected:
    bool eventFilter(QObject* ptrObject, QEvent* ptrEvent) override;

private:
    //void updateConfiguration();
    //void setConfiguration(const C64KeyboardConfiguration& configuration);
    //const C64KeyboardMapping* mapping(quint32 nativeScanCode) const;

private:
    ConfigurationManager* m_ptrConfigurationManager = nullptr;
};
