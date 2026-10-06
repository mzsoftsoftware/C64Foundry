#pragma once

#include <QObject>

#include "Configuration/C64KeyboardConfiguration.h"
#include "Input/InputEvent.h"


class KeyboardController : public QObject
{
    Q_OBJECT
public:
    explicit KeyboardController(QObject* parent = nullptr);
    virtual ~KeyboardController();

    // Setter
    void setConfiguration(const C64KeyboardConfiguration& configuration);

public slots:
    void keyPressed(int key, const quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);
    void keyReleased(int key, const quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);

signals:
    void input(const InputEvent& event);

protected:
    bool eventFilter(QObject* ptrObject, QEvent* ptrEvent) override;

private:
    const C64KeyboardMapping* mapping(quint32 nativeScanCode) const;

private:
    C64KeyboardConfiguration m_configuration;
};
