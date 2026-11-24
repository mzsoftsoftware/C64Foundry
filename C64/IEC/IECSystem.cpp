#include "IECSystem.h"

#include "IECDeviceFactory.h"
#include "IECDevice.h"
#include "IECDeviceRequest.h"
#include "IECSystemRequest.h"


IECSystem::IECSystem(C64Machine* ptrMachine, IECDeviceFactory* ptrDeviceFactory)
    : m_ptrMachine(ptrMachine),
    m_ptrDeviceFactory(ptrDeviceFactory)
{
}
IECSystem::~IECSystem()
{
}

QList<IECDeviceType> IECSystem::availableDeviceTypes() const
{
    if (m_ptrDeviceFactory == nullptr)
        return {};

    return m_ptrDeviceFactory->availableDeviceTypes();
}
QList<quint8> IECSystem::supportedDeviceNumbers(const IECDeviceType type) const
{
    if (m_ptrDeviceFactory == nullptr)
        return {};

    return m_ptrDeviceFactory->supportedDeviceNumbers(type);
}


IECResponse::Result IECSystem::processRequest(const IECRequest& request, IECResponse& response)
{
    const IECSystemRequest* ptrSystemRequest = dynamic_cast<const IECSystemRequest*>(&request);

    if (ptrSystemRequest != nullptr)
    {
        IECSystemResponse* ptrSystemResponse = dynamic_cast<IECSystemResponse*>(&response);

        if (ptrSystemResponse == nullptr)
            return IECResponse::Result::InvalidParameter;

        if (!ptrSystemRequest->execute(*this, *ptrSystemResponse))
            return IECResponse::Result::Failed;

        return IECResponse::Result::Success;
    }

    const IECDeviceRequest* ptrDeviceRequest = dynamic_cast<const IECDeviceRequest*>(&request);

    if (ptrDeviceRequest != nullptr)
    {
        IECDeviceResponse* ptrDeviceResponse = dynamic_cast<IECDeviceResponse*>(&response);

        if (ptrDeviceResponse == nullptr)
            return IECResponse::Result::InvalidParameter;

        DeviceEntry* ptrEntry = findEntry(ptrDeviceRequest->deviceId());

        if (ptrEntry == nullptr || ptrEntry->ptrDevice == nullptr)
            return IECResponse::Result::InvalidParameter;

        if (!ptrDeviceRequest->execute(*ptrEntry->ptrDevice, *ptrDeviceResponse))
            return IECResponse::Result::Failed;

        return IECResponse::Result::Success;
    }

    return IECResponse::Result::InvalidParameter;
}
