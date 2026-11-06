#pragma once


class IECDeviceFactory
{
public:
    IECDeviceFactory() = default;
    ~IECDeviceFactory() = default;

    IECDeviceFactory(const IECDeviceFactory&) = delete;
    IECDeviceFactory& operator=(const IECDeviceFactory&) = delete;
};
