#include "IECSystem.h"


IECSystem::IECSystem(C64Machine* ptrMachine, IECDeviceFactory* ptrDeviceFactory)
    : m_ptrMachine(ptrMachine),
    m_ptrDeviceFactory(ptrDeviceFactory)
{
}

IECSystem::~IECSystem()
{
}