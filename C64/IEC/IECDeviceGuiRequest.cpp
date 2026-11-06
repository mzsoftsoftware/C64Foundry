#include "IECDeviceGuiRequest.h"

#include "IECPrinter.h"


bool IECPrinterTearOffRequest::execute(IECDevice& device, IECDeviceGuiResponse& response) const
{
    IECPrinter* ptrPrinter = dynamic_cast<IECPrinter*>(&device);
    IECPrinterTearOffResponse* ptrResponse =
        dynamic_cast<IECPrinterTearOffResponse*>(&response);

    if (ptrPrinter == nullptr || ptrResponse == nullptr)
        return false;

    ptrResponse->deviceId = deviceId;

    return ptrPrinter->tearOff(*ptrResponse);
}