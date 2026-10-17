#include "C64CIA1Port.h"

#include "C64/Input/C64Keyboard.h"


C64CIA1Port::C64CIA1Port()
{
}
C64CIA1Port::~C64CIA1Port()
{
}

quint8 C64CIA1Port::portAInputs(const quint8 portBOutput) const
{
    return m_ptrKeyboard->portAInputs(portBOutput);
}

quint8 C64CIA1Port::portBInputs(const quint8 portAOutput) const
{
    return m_ptrKeyboard->portBInputs(portAOutput);
}
