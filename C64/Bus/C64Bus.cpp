#include "C64Bus.h"

C64Bus::C64Bus()
{
}
C64Bus::~C64Bus()
{
}

void C64Bus::setMemory(C64Memory* ptrMemory)
{
    m_ptrMemory = ptrMemory;
}