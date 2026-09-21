#include "MOS6510.h"


MOS6510::MOS6510()
{
}
MOS6510::~MOS6510()
{
}

void MOS6510::setBus(C64Bus* ptrBus)
{
    m_ptrBus = ptrBus;
}