#include "IECSystem.h"

#include "IECDeviceFactory.h"


IECSystem::IECSystem(C64Machine* ptrMachine, IECDeviceFactory* ptrDeviceFactory)
    : m_ptrMachine(ptrMachine),
    m_ptrDeviceFactory(ptrDeviceFactory)
{
}
IECSystem::~IECSystem()
{
}

QList<quint8> IECSystem::supportedDeviceNumbers(const IECDeviceType type) const
{
    if (m_ptrDeviceFactory == nullptr)
        return {};

    return m_ptrDeviceFactory->supportedDeviceNumbers(type);
}
