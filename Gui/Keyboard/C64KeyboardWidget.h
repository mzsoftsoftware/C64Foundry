#pragma once

#include <QTransform>
#include <QWidget>

#include "C64KeyboardLayout.h"

class QMouseEvent;
class QPaintEvent;
class QEvent;


class C64KeyboardWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Mode
    {
        Input,
        Editor
    };

    explicit C64KeyboardWidget(QWidget* parent = nullptr);
    virtual ~C64KeyboardWidget() override;

    // Setter
    void setMode(Mode mode);

    // Getter
    Mode mode() const                                       { return m_mode; }

signals:
    void keyPressed(C64Key key);
    void keyReleased(C64Key key);

protected:
    bool event(QEvent* ptrEvent) override;
    void paintEvent(QPaintEvent* ptrEvent) override;
    void leaveEvent(QEvent* ptrEvent) override;
    void mouseMoveEvent(QMouseEvent* ptrEvent) override;
    void mousePressEvent(QMouseEvent* ptrEvent) override;
    void mouseReleaseEvent(QMouseEvent* ptrEvent) override;

private:
    void setKeyboardKeyPressed(C64Key key, bool bPressed);
    bool isModifierKey(C64Key key) const;
    void setModifiers(const C64KeyboardModifiers& modifiers);
    void setModifierLatched(C64KeyboardKey* ptrKey, bool bLatched);
    void releaseTemporaryModifiers();
    void cancelMouseKey();

private:
    QTransform keyboardTransform() const;
    QPointF mapToKeyboard(const QPointF& position) const;

    void updateKey(const C64KeyboardKey* ptrKey);
    void setHoveredKey(C64KeyboardKey* ptrKey);
    void releaseMouseKey();

private:
    Mode m_mode = Mode::Input;

    C64KeyboardLayout m_layout;
    C64KeyboardKey* m_ptrHoveredKey = nullptr;
    C64KeyboardKey* m_ptrMousePressedKey = nullptr;
};
