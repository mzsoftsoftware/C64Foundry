
#include "IECDeviceRequest.h"

#include "IECDevice.h"
#include "IECPrinter.h"
#include "IECFoundryDrive.h"
#include "IECFoundryDisk.h"


bool IECPrinterTearOffRequest::execute(IECDevice& device, IECDeviceResponse& response) const
{
    IECPrinter* ptrPrinter = dynamic_cast<IECPrinter*>(&device);
    IECPrinterTearOffResponse* ptrResponse =
        dynamic_cast<IECPrinterTearOffResponse*>(&response);

    if (ptrPrinter == nullptr || ptrResponse == nullptr)
        return false;

    ptrResponse->setDeviceId(deviceId());

    ptrPrinter->tearOff();

    return true;
}


bool IECFoundryDriveSetDiskEnabledRequest::execute(
    IECDevice& device, IECDeviceResponse& response) const
{
    IECFoundryDrive* ptrDrive = dynamic_cast<IECFoundryDrive*>(&device);
    IECFoundryDriveResponse* ptrResponse =
        dynamic_cast<IECFoundryDriveResponse*>(&response);

    if (ptrDrive == nullptr || ptrResponse == nullptr)
        return false;

    ptrResponse->setDeviceId(deviceId());

    if (!ptrDrive->setDiskEnabled(deviceNumber(), enabled()))
    {
        ptrResponse->setErrorMessage(
            QStringLiteral("Disk slot konnte nicht geändert werden."));
        return false;
    }

    return true;
}


bool IECFoundryDriveInsertImageRequest::execute(
    IECDevice& device, IECDeviceResponse& response) const
{
    IECFoundryDrive* ptrDrive = dynamic_cast<IECFoundryDrive*>(&device);
    IECFoundryDriveResponse* ptrResponse =
        dynamic_cast<IECFoundryDriveResponse*>(&response);

    if (ptrDrive == nullptr || ptrResponse == nullptr)
        return false;

    ptrResponse->setDeviceId(deviceId());

    IECFoundryDisk* ptrDisk = ptrDrive->diskByDeviceNumber(deviceNumber());

    if (ptrDisk == nullptr)
    {
        ptrResponse->setErrorMessage(
            QStringLiteral("Disk slot wurde nicht gefunden."));
        return false;
    }

    if (!ptrDrive->insertImage(deviceNumber(), fileName()))
    {
        ptrResponse->setErrorMessage(ptrDisk->lastError());
        return false;
    }

    return true;
}


bool IECFoundryDriveEjectImageRequest::execute(
    IECDevice& device, IECDeviceResponse& response) const
{
    IECFoundryDrive* ptrDrive = dynamic_cast<IECFoundryDrive*>(&device);
    IECFoundryDriveResponse* ptrResponse =
        dynamic_cast<IECFoundryDriveResponse*>(&response);

    if (ptrDrive == nullptr || ptrResponse == nullptr)
        return false;

    ptrResponse->setDeviceId(deviceId());

    IECFoundryDisk* ptrDisk = ptrDrive->diskByDeviceNumber(deviceNumber());

    if (ptrDisk == nullptr)
    {
        ptrResponse->setErrorMessage(
            QStringLiteral("Disk slot wurde nicht gefunden."));
        return false;
    }

    if (!ptrDrive->ejectImage(deviceNumber()))
    {
        ptrResponse->setErrorMessage(ptrDisk->lastError());
        return false;
    }

    return true;
}
