#include "C64Keyboard.h"


C64Keyboard::C64Keyboard()
    : m_matrixToPortA{}
    , m_matrixToPortB{}
{
}

void C64Keyboard::press(const C64Key key)
{
    C64Key matrixKey = key;

    if (key == C64Key::LeftShift)
    {
        m_leftShiftPressed = true;
    }
    else if (key == C64Key::ShiftLock)
    {
        m_shiftLockPressed = true;
        matrixKey = C64Key::LeftShift;
    }

    const quint8 matrixCode = static_cast<quint8>(matrixKey);
    if (matrixCode >= 64)
        return;
    const quint8 row = matrixCode >> 3;
    const quint8 column = matrixCode & 0x07;
    m_matrixToPortB[row] |= static_cast<quint8>(1U << column);
    m_matrixToPortA[column] |= static_cast<quint8>(1U << row);
}

void C64Keyboard::release(const C64Key key)
{
    C64Key matrixKey = key;

    if (key == C64Key::LeftShift)
    {
        m_leftShiftPressed = false;
        if (m_shiftLockPressed)
            return;
    }
    else if (key == C64Key::ShiftLock)
    {
        m_shiftLockPressed = false;
        if (m_leftShiftPressed)
            return;
        matrixKey = C64Key::LeftShift;
    }

    const quint8 matrixCode = static_cast<quint8>(matrixKey);
    if (matrixCode >= 64)
        return;
    const quint8 row = matrixCode >> 3;
    const quint8 column = matrixCode & 0x07;
    m_matrixToPortB[row] &= static_cast<quint8>(~(1U << column));
    m_matrixToPortA[column] &= static_cast<quint8>(~(1U << row));
}

quint8 C64Keyboard::portAInputs(const quint8 portBPins) const
{
    quint8 inputs = 0xFF;
    for (quint8 row = 0; row < 8; ++row)
    {
        if ((portBPins & (1U << row)) == 0)
        {
            inputs &= static_cast<quint8>(~m_matrixToPortB[row]);
        }
    }
    return inputs;
}

quint8 C64Keyboard::portBInputs(const quint8 portAPins) const
{
    quint8 inputs = 0xFF;
    for (quint8 column = 0; column < 8; ++column)
    {
        if ((portAPins & (1U << column)) == 0)
        {
            inputs &= static_cast<quint8>(~m_matrixToPortA[column]);
        }
    }
    return inputs;
}
