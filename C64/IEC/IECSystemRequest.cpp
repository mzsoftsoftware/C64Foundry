#include "IECSystemRequest.h"

#include "IECSystem.h"


bool IECSystemGetConfigurationsRequest::execute(IECSystem& system, IECSystemResponse& response) const
{
    IECSystemGetConfigurationsResponse* ptrResponse =
        dynamic_cast<IECSystemGetConfigurationsResponse*>(&response);

    if (ptrResponse == nullptr)
        return false;

    ptrResponse->setConfigurations(system.configurations());
    ptrResponse->setAvailableDeviceTypes(system.availableDeviceTypes());
    ptrResponse->setAddableDeviceTypes(system.addableDeviceTypes());

    for (IECDeviceType type : system.availableDeviceTypes())
        ptrResponse->setSupportedDeviceNumbers(type, system.supportedDeviceNumbers(type));

    return true;
}


bool IECSystemAddDeviceRequest::execute(IECSystem& system, IECSystemResponse& response) const
{
    IECSystemAddDeviceResponse* ptrResponse =
        dynamic_cast<IECSystemAddDeviceResponse*>(&response);

    if (ptrResponse == nullptr)
        return false;

    const IECDeviceConfiguration::DeviceId deviceId = system.addDevice(deviceType());

    if (deviceId == 0)
        return false;

    ptrResponse->setDeviceId(deviceId);

    return true;
}


bool IECSystemRemoveDeviceRequest::execute(IECSystem& system, IECSystemResponse& response) const
{
    IECSystemRemoveDeviceResponse* ptrResponse =
        dynamic_cast<IECSystemRemoveDeviceResponse*>(&response);

    if (ptrResponse == nullptr)
        return false;

    if (!system.removeDevice(deviceId()))
        return false;

    ptrResponse->setDeviceId(deviceId());

    return true;
}


bool IECSystemSetConfigurationRequest::execute(IECSystem& system, IECSystemResponse& response) const
{
    IECSystemSetConfigurationResponse* ptrResponse =
        dynamic_cast<IECSystemSetConfigurationResponse*>(&response);

    if (ptrResponse == nullptr)
        return false;

    QString operationMessage;

    const bool success = system.setConfiguration(configuration(), &operationMessage);

    ptrResponse->setDeviceId(configuration().id());
    ptrResponse->setOperationMessage(operationMessage);

    return success;
}


bool IECSystemRestoreConfigurationsRequest::execute(IECSystem& system, IECSystemResponse& response) const
{
    if (dynamic_cast<IECSystemRestoreConfigurationsResponse*>(&response) == nullptr)
        return false;

    return system.restoreConfigurations(configurations());
}
