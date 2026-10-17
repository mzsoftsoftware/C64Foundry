#pragma once

#include "C64/CIA/MOS6526Port.h"

class C64Keyboard;


class C64CIA1Port : public MOS6526Port
{
public:
    explicit C64CIA1Port();
    virtual ~C64CIA1Port() override;

    // Setter
    void setKeyboard(C64Keyboard* ptrKeyboard)          { m_ptrKeyboard = ptrKeyboard; }

    // Operations
    quint8 portAInputs(quint8 portBOutput) const override;
    quint8 portBInputs(quint8 portAOutput) const override;

private:
    C64Keyboard* m_ptrKeyboard = nullptr;
};
