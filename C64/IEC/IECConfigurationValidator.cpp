#include "IECConfigurationValidator.h"

#include "IECAsciiPrinterConfiguration.h"
#include "IECFoundryDriveConfiguration.h"
#include "IECFoundryDiskConfiguration.h"

#include <QSet>


namespace
{

constexpr int MinimumIECDeviceNumber = 4;
constexpr int MaximumIECDeviceNumber = 30;


void appendError(QStringList* errors, const QString& error)
{
    if (errors != nullptr)
        errors->append(error);
}


bool registerAddress(int deviceNumber, QSet<int>& usedAddresses, QStringList* errors, const QString& deviceDescription)
{
    if (deviceNumber < MinimumIECDeviceNumber || deviceNumber > MaximumIECDeviceNumber)
    {
        appendError(errors, QStringLiteral("%1 uses invalid IEC device number %2.")
                        .arg(deviceDescription)
                        .arg(deviceNumber));
        return false;
    }

    if (usedAddresses.contains(deviceNumber))
    {
        appendError(errors, QStringLiteral("IEC device number %1 is assigned more than once.")
                        .arg(deviceNumber));
        return false;
    }

    usedAddresses.insert(deviceNumber);
    return true;
}


}


bool IECConfigurationValidator::validate(
    const QList<QSharedPointer<IECDeviceConfiguration>>& configurations,
    QStringList* errors)
{
    if (errors != nullptr)
        errors->clear();

    bool valid = true;

    QSet<IECDeviceConfiguration::DeviceId> usedIds;
    QSet<int> usedAddresses;

    for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
        {
            appendError(errors, QStringLiteral("IEC device configuration is null."));
            valid = false;
            continue;
        }

        const IECDeviceConfiguration& configuration = *ptrConfiguration;

        //
        // Device ID
        //
        if (configuration.id() == 0)
        {
            appendError(errors, QStringLiteral("IEC device has an invalid DeviceId (0)."));
            valid = false;
        }
        else if (usedIds.contains(configuration.id()))
        {
            appendError(errors, QStringLiteral("IEC DeviceId %1 is assigned more than once.")
                            .arg(configuration.id()));
            valid = false;
        }
        else
        {
            usedIds.insert(configuration.id());
        }

        //
        // Device type and concrete configuration
        //
        switch (configuration.type())
        {
        case IECDeviceType::PrinterASCII:
        {
            const IECAsciiPrinterConfiguration* ptrPrinterConfiguration =
                dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

            if (ptrPrinterConfiguration == nullptr)
            {
                appendError(errors, QStringLiteral("IEC device %1 has an invalid configuration type for PrinterASCII.")
                                .arg(configuration.id()));
                valid = false;
                break;
            }

            //
            // IEC address
            //
            if (!registerAddress(
                    ptrPrinterConfiguration->deviceNumber(),
                    usedAddresses,
                    errors,
                    QStringLiteral("ASCII printer %1").arg(configuration.id())))
            {
                valid = false;
            }

            //
            // Printer settings
            //
            if (ptrPrinterConfiguration->lineWidth() < 1)
            {
                appendError(errors, QStringLiteral("ASCII printer %1 has an invalid line width.")
                                .arg(configuration.id()));
                valid = false;
            }

            break;
        }

        case IECDeviceType::FoundryDrive:
        {
            const IECFoundryDriveConfiguration* ptrDriveConfiguration =
                dynamic_cast<const IECFoundryDriveConfiguration*>(&configuration);

            if (ptrDriveConfiguration == nullptr)
            {
                appendError(errors, QStringLiteral("IEC device %1 has an invalid configuration type for FoundryDrive.")
                                .arg(configuration.id()));
                valid = false;
                break;
            }

            const int startDeviceNumber = ptrDriveConfiguration->startDeviceNumber();
            const int slotCount = ptrDriveConfiguration->slotCount();
            const int endDeviceNumber = ptrDriveConfiguration->endDeviceNumber();

            const bool slotCountValid =
                slotCount >= 1 &&
                slotCount <= IECFoundryDriveConfiguration::MaximumDiskCount;

            const bool startAddressValid =
                startDeviceNumber >= MinimumIECDeviceNumber &&
                startDeviceNumber <= MaximumIECDeviceNumber;

            if (!slotCountValid)
            {
                appendError(errors, QStringLiteral("Foundry Drive %1 has an invalid slot count of %2.")
                                .arg(configuration.id())
                                .arg(slotCount));
                valid = false;
            }

            if (!startAddressValid)
            {
                appendError(errors, QStringLiteral("Foundry Drive %1 has an invalid start IEC device number of %2.")
                                .arg(configuration.id())
                                .arg(startDeviceNumber));
                valid = false;
            }

            if (slotCountValid && startAddressValid)
            {
                if (endDeviceNumber > MaximumIECDeviceNumber)
                {
                    appendError(errors, QStringLiteral("Foundry Drive %1 reserves an IEC address range ending at %2, which exceeds the maximum address %3.")
                                    .arg(configuration.id())
                                    .arg(endDeviceNumber)
                                    .arg(MaximumIECDeviceNumber));
                    valid = false;
                }
                else
                {
                    //
                    // Reserve every address, including empty disk slots.
                    //
                    for (int deviceNumber = startDeviceNumber;
                         deviceNumber <= endDeviceNumber;
                         ++deviceNumber)
                    {
                        if (!registerAddress(
                                deviceNumber,
                                usedAddresses,
                                errors,
                                QStringLiteral("Foundry Drive %1").arg(configuration.id())))
                        {
                            valid = false;
                        }
                    }
                }
            }

            //
            // Disk configurations
            //
            QSet<int> usedSlotIndices;

            for (const IECFoundryDiskConfiguration& diskConfiguration : ptrDriveConfiguration->disks())
            {
                const int slotIndex = diskConfiguration.slotIndex();

                if (slotIndex < 0 || slotIndex >= slotCount)
                {
                    appendError(errors, QStringLiteral("Foundry Drive %1 has a disk configuration with invalid slot index %2.")
                                    .arg(configuration.id())
                                    .arg(slotIndex));
                    valid = false;
                }

                if (usedSlotIndices.contains(slotIndex))
                {
                    appendError(errors, QStringLiteral("Foundry Drive %1 has multiple disk configurations for slot %2.")
                                    .arg(configuration.id())
                                    .arg(slotIndex));
                    valid = false;
                }
                else
                {
                    usedSlotIndices.insert(slotIndex);
                }
            }

            break;
        }

        default:
            appendError(errors, QStringLiteral("IEC device %1 has an unknown device type.")
                                    .arg(configuration.id()));
            valid = false;
            break;
        }
    }

    return valid;
}
