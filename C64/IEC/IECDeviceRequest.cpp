#include "IECDeviceRequest.h"

#include "IECDevice.h"
#include "IECPrinter.h"


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
