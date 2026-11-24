#include "IECSystem.h"

#include "IECDeviceFactory.h"
#include "IECDevice.h"
#include "IECDeviceRequest.h"

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


IECResponse::Result IECSystem::processRequest(const IECRequest& request, IECResponse& response)
{
    const IECDeviceRequest* ptrDeviceRequest = dynamic_cast<const IECDeviceRequest*>(&request);
    IECDeviceResponse* ptrDeviceResponse = dynamic_cast<IECDeviceResponse*>(&response);

    if (ptrDeviceRequest == nullptr || ptrDeviceResponse == nullptr)
        return IECResponse::Result::InvalidParameter;

    DeviceEntry* ptrEntry = findEntry(ptrDeviceRequest->deviceId());

    if (ptrEntry == nullptr || ptrEntry->ptrDevice == nullptr)
        return IECResponse::Result::InvalidParameter;

    if (!ptrDeviceRequest->execute(*ptrEntry->ptrDevice, *ptrDeviceResponse))
        return IECResponse::Result::Failed;

    return IECResponse::Result::Success;
}
