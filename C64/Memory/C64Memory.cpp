#include "C64Memory.h"

#include "RAM.h"
#include "BasicROM.h"
#include "KernalROM.h"
#include "CharROM.h"
#include "ColorRAM.h"


C64Memory::C64Memory()
{
    m_ptrRAM = new RAM();
    m_ptrBasicROM = new BasicROM();
    m_ptrKernalROM = new KernalROM();
    m_ptrCharROM = new CharROM();
    m_ptrColorRAM = new ColorRAM();
}
C64Memory::~C64Memory()
{
}
