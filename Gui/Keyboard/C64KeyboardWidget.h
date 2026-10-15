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

    // Operations
    void setKeyboardKeyPressed(C64Key key, bool bPressed);
    void setModifiers(const C64KeyboardModifiers& modifiers);

protected:
    void paintEvent(QPaintEvent* ptrEvent) override;
    void mouseMoveEvent(QMouseEvent* ptrEvent) override;
    void leaveEvent(QEvent* ptrEvent) override;

private:
    QTransform keyboardTransform() const;
    QPointF mapToKeyboard(const QPointF& position) const;

    void updateKey(const C64KeyboardKey* ptrKey);
    void setHoveredKey(C64KeyboardKey* ptrKey);

private:
    Mode m_mode = Mode::Input;

    C64KeyboardLayout m_layout;
    C64KeyboardKey* m_ptrHoveredKey = nullptr;
};
