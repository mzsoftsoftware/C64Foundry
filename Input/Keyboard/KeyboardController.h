#pragma once

#include <QObject>

#include "C64/Input/C64Keyboard.h"
#include "Input/InputEvent.h"


class KeyboardController : public QObject
{
    Q_OBJECT
public:
    explicit KeyboardController(QObject* parent = nullptr);
    virtual ~KeyboardController();

public slots:
    void keyPressed(int key, const quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);
    void keyReleased(int key, const quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);
    void inputDeactivated();

signals:
    void input(const InputEvent& event);

protected:
    bool eventFilter(QObject* ptrObject, QEvent* ptrEvent) override;

private:
    bool mapKey(int key, quint32 nativeScanCode, C64Key& c64Key) const;
};
