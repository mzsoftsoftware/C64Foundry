#include "ConfigurationStorage.h"

#include <QSettings>
#include <QStringList>

#include "C64Configuration.h"
#include "C64/IEC/IECAsciiPrinterConfiguration.h"
#include "C64/IEC/IECFoundryDriveConfiguration.h"
#include "C64/IEC/IECFoundryDiskConfiguration.h"
#include "C64/IEC/IECConfigurationValidator.h"


bool ConfigurationStorage::load(QMap<QString, C64Configuration*>& configurations, QString& activeConfigurationName) const
{
    QSettings settings;

    activeConfigurationName = settings.value(QStringLiteral("ActiveConfiguration")).toString();
    settings.beginGroup(QStringLiteral("Configurations"));

    const QStringList configurationNames = settings.childGroups();
    for (const QString& configurationName : configurationNames)
    {
        settings.beginGroup(configurationName);

        C64Configuration* ptrConfiguration = new C64Configuration();
        ptrConfiguration->name = configurationName;
        ptrConfiguration->platform.setName(settings.value(QStringLiteral("Platform")).toString());

        //
        // ROM set
        //
        settings.beginGroup(QStringLiteral("ROMSet"));
        ptrConfiguration->romSet.basicROMFileName = settings.value(QStringLiteral("Basic")).toString();
        ptrConfiguration->romSet.kernalROMFileName = settings.value(QStringLiteral("Kernal")).toString();
        ptrConfiguration->romSet.characterROMFileName = settings.value(QStringLiteral("Character")).toString();
        settings.endGroup();

        //
        // Keyboard
        //
        settings.beginGroup(QStringLiteral("Keyboard"));
        if (!loadKeyboard(settings, ptrConfiguration->keyboard))
        {
            settings.endGroup();
            settings.endGroup();
            settings.endGroup();

            delete ptrConfiguration;
            qDeleteAll(configurations);
            configurations.clear();

            return false;
        }
        settings.endGroup();

        //
        // IEC devices
        //
        settings.beginGroup(QStringLiteral("IEC"));
        if (!loadIECDevices(settings, ptrConfiguration->iecDevices))
        {
            settings.endGroup();
            settings.endGroup();
            settings.endGroup();

            delete ptrConfiguration;
            qDeleteAll(configurations);
            configurations.clear();

            return false;
        }
        settings.endGroup();

        settings.endGroup();

        configurations.insert(configurationName, ptrConfiguration);
    }

    settings.endGroup();

    return settings.status() == QSettings::NoError;
}

bool ConfigurationStorage::save(const QMap<QString, C64Configuration*>& configurations, const QString& activeConfigurationName) const
{
    //
    // Validate all IEC configurations before changing the stored configuration.
    //
    for (C64Configuration* ptrConfiguration : configurations)
    {
        if (ptrConfiguration == nullptr)
            return false;

        QStringList errors;
        if (!IECConfigurationValidator::validate(ptrConfiguration->iecDevices, &errors))
            return false;
    }

    QSettings settings;

    settings.setValue(QStringLiteral("ActiveConfiguration"), activeConfigurationName);
    settings.beginGroup(QStringLiteral("Configurations"));
    settings.remove(QString());

    for (C64Configuration* ptrConfiguration : configurations)
    {
        settings.beginGroup(ptrConfiguration->name);
        settings.setValue(QStringLiteral("Platform"), ptrConfiguration->platform.name());

        //
        // ROM set
        //
        settings.beginGroup(QStringLiteral("ROMSet"));
        settings.setValue(QStringLiteral("Basic"), ptrConfiguration->romSet.basicROMFileName);
        settings.setValue(QStringLiteral("Kernal"), ptrConfiguration->romSet.kernalROMFileName);
        settings.setValue(QStringLiteral("Character"), ptrConfiguration->romSet.characterROMFileName);
        settings.endGroup();

        //
        // Keyboard
        //
        settings.beginGroup(QStringLiteral("Keyboard"));
        saveKeyboard(settings, ptrConfiguration->keyboard);
        settings.endGroup();

        //
        // IEC devices
        //
        settings.beginGroup(QStringLiteral("IEC"));
        if (!saveIECDevices(settings, ptrConfiguration->iecDevices))
        {
            settings.endGroup();
            settings.endGroup();
            settings.endGroup();

            return false;
        }
        settings.endGroup();

        settings.endGroup();
    }

    settings.endGroup();
    settings.sync();

    return settings.status() == QSettings::NoError;
}

bool ConfigurationStorage::loadKeyboard(QSettings& settings, C64KeyboardConfiguration& keyboard) const
{
    const int overrideCount = settings.beginReadArray(QStringLiteral("Overrides"));

    for (int index = 0; index < overrideCount; ++index)
    {
        settings.setArrayIndex(index);

        //
        // Native scan code
        //
        bool nativeScanCodeValid = false;
        const quint32 nativeScanCode = settings.value(QStringLiteral("NativeScanCode")).toUInt(&nativeScanCodeValid);
        if (!nativeScanCodeValid)
        {
            settings.endArray();
            return false;
        }

        //
        // Mapping mode
        //
        C64KeyboardMappingMode mode;
        if (!C64KeyboardConfiguration::mappingModeFromName(settings.value(QStringLiteral("Mode")).toString(), mode))
        {
            settings.endArray();
            return false;
        }

        //
        // C64 keys
        //
        QList<C64Key> keys;
        const QStringList keyNames = settings.value(QStringLiteral("Keys")).toStringList();

        for (const QString& keyName : keyNames)
        {
            C64Key key;
            if (!C64KeyboardConfiguration::keyFromName(keyName, key))
            {
                settings.endArray();
                return false;
            }

            keys.append(key);
        }

        //
        // Register override
        //
        keyboard.setOverride({ nativeScanCode, keys, mode });
    }

    settings.endArray();

    return true;
}

void ConfigurationStorage::saveKeyboard(QSettings& settings, const C64KeyboardConfiguration& keyboard) const
{
    const QList<C64KeyboardMapping>& keyboardOverrides = keyboard.overrides();

    settings.beginWriteArray(QStringLiteral("Overrides"), keyboardOverrides.size());

    for (int index = 0; index < keyboardOverrides.size(); ++index)
    {
        settings.setArrayIndex(index);

        const C64KeyboardMapping& mapping = keyboardOverrides[index];

        //
        // C64 keys
        //
        QStringList keyNames;
        for (const C64Key key : mapping.keys)
            keyNames.append(C64KeyboardConfiguration::keyName(key));

        //
        // Store mapping
        //
        settings.setValue(QStringLiteral("NativeScanCode"), mapping.nativeScanCode);
        settings.setValue(QStringLiteral("Keys"), keyNames);
        settings.setValue(QStringLiteral("Mode"), C64KeyboardConfiguration::mappingModeName(mapping.mode));
    }

    settings.endArray();
}

bool ConfigurationStorage::loadIECDevices(QSettings& settings, QList<QSharedPointer<IECDeviceConfiguration>>& devices) const
{
    QList<QSharedPointer<IECDeviceConfiguration>> loadedDevices;

    const int deviceCount = settings.beginReadArray(QStringLiteral("Devices"));

    for (int index = 0; index < deviceCount; ++index)
    {
        settings.setArrayIndex(index);

        bool idValid = false;
        const IECDeviceConfiguration::DeviceId id = settings.value(QStringLiteral("Id")).toULongLong(&idValid);
        const QString typeName = settings.value(QStringLiteral("Type")).toString();

        if (!idValid || id == 0)
        {
            settings.endArray();
            return false;
        }

        QSharedPointer<IECDeviceConfiguration> ptrConfiguration;

        if (typeName == QStringLiteral("PrinterASCII"))
        {
            bool deviceNumberValid = false;
            const uint deviceNumber = settings.value(QStringLiteral("DeviceNumber")).toUInt(&deviceNumberValid);

            bool lineWidthValid = false;
            const int lineWidth = settings.value(QStringLiteral("LineWidth"), 80).toInt(&lineWidthValid);

            if (!deviceNumberValid || deviceNumber > 255 || !lineWidthValid || lineWidth < 1)
            {
                settings.endArray();
                return false;
            }

            IECAsciiPrinterConfiguration* ptrPrinterConfiguration = new IECAsciiPrinterConfiguration();

            ptrPrinterConfiguration->setDeviceNumber(static_cast<quint8>(deviceNumber));
            ptrPrinterConfiguration->setLineWidth(lineWidth);
            ptrPrinterConfiguration->setAutoWrap(settings.value(QStringLiteral("AutoWrap"), true).toBool());

            ptrConfiguration.reset(ptrPrinterConfiguration);
        }
        else if (typeName == QStringLiteral("FoundryDrive"))
        {
            bool startDeviceNumberValid = false;
            const uint startDeviceNumber = settings.value(QStringLiteral("StartDeviceNumber")).toUInt(&startDeviceNumberValid);

            bool slotCountValid = false;
            const int slotCount = settings.value(QStringLiteral("SlotCount")).toInt(&slotCountValid);

            if (!startDeviceNumberValid || startDeviceNumber > 255 || !slotCountValid)
            {
                settings.endArray();
                return false;
            }

            IECFoundryDriveConfiguration* ptrDriveConfiguration = new IECFoundryDriveConfiguration();

            ptrDriveConfiguration->setStartDeviceNumber(static_cast<quint8>(startDeviceNumber));
            ptrDriveConfiguration->setSlotCount(slotCount);

            //
            // Load configured disk slots.
            //
            const int diskCount = settings.beginReadArray(QStringLiteral("Disks"));

            for (int diskIndex = 0; diskIndex < diskCount; ++diskIndex)
            {
                settings.setArrayIndex(diskIndex);

                bool slotIndexValid = false;
                const int slotIndex = settings.value(QStringLiteral("SlotIndex")).toInt(&slotIndexValid);

                if (!slotIndexValid)
                {
                    settings.endArray();
                    settings.endArray();
                    return false;
                }

                IECFoundryDiskConfiguration diskConfiguration;
                diskConfiguration.setSlotIndex(slotIndex);
                diskConfiguration.setEnabled(settings.value(QStringLiteral("Enabled"), false).toBool());

                const bool saveCurrentDiskOnExit = settings.value(QStringLiteral("SaveCurrentDiskOnExit"), false).toBool();
                diskConfiguration.setSaveCurrentDiskOnExit(saveCurrentDiskOnExit);

                //
                // The image association is persisted only when requested.
                // Image contents are handled by the drive independently.
                //
                if (saveCurrentDiskOnExit)
                    diskConfiguration.setImageFileName(settings.value(QStringLiteral("ImageFileName")).toString());

                if (!ptrDriveConfiguration->addDiskConfiguration(diskConfiguration))
                {
                    settings.endArray();
                    settings.endArray();
                    return false;
                }
            }

            settings.endArray();

            ptrConfiguration.reset(ptrDriveConfiguration);
        }
        else
        {
            //
            // Unknown types, including the obsolete FoundryDisk device type,
            // are not migrated implicitly.
            //
            settings.endArray();
            return false;
        }

        ptrConfiguration->setId(id);
        ptrConfiguration->setEnabled(settings.value(QStringLiteral("Enabled"), false).toBool());

        loadedDevices.append(ptrConfiguration);
    }

    settings.endArray();

    QStringList errors;
    if (!IECConfigurationValidator::validate(loadedDevices, &errors))
        return false;

    devices = loadedDevices;

    return true;
}

bool ConfigurationStorage::saveIECDevices(QSettings& settings, const QList<QSharedPointer<IECDeviceConfiguration>>& devices) const
{
    settings.beginWriteArray(QStringLiteral("Devices"), devices.size());

    for (int index = 0; index < devices.size(); ++index)
    {
        settings.setArrayIndex(index);

        const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration = devices[index];

        if (ptrConfiguration.isNull())
        {
            settings.endArray();
            return false;
        }

        settings.setValue(QStringLiteral("Id"), static_cast<qulonglong>(ptrConfiguration->id()));
        settings.setValue(QStringLiteral("Enabled"), ptrConfiguration->enabled());

        switch (ptrConfiguration->type())
        {
        case IECDeviceType::PrinterASCII:
        {
            const IECAsciiPrinterConfiguration* ptrPrinterConfiguration = dynamic_cast<const IECAsciiPrinterConfiguration*>(ptrConfiguration.data());

            if (ptrPrinterConfiguration == nullptr)
            {
                settings.endArray();
                return false;
            }

            settings.setValue(QStringLiteral("Type"), QStringLiteral("PrinterASCII"));
            settings.setValue(QStringLiteral("DeviceNumber"), static_cast<uint>(ptrPrinterConfiguration->deviceNumber()));
            settings.setValue(QStringLiteral("LineWidth"), ptrPrinterConfiguration->lineWidth());
            settings.setValue(QStringLiteral("AutoWrap"), ptrPrinterConfiguration->autoWrap());

            break;
        }
        case IECDeviceType::FoundryDrive:
        {
            const IECFoundryDriveConfiguration* ptrDriveConfiguration = dynamic_cast<const IECFoundryDriveConfiguration*>(ptrConfiguration.data());

            if (ptrDriveConfiguration == nullptr)
            {
                settings.endArray();
                return false;
            }

            settings.setValue(QStringLiteral("Type"), QStringLiteral("FoundryDrive"));
            settings.setValue(QStringLiteral("StartDeviceNumber"), static_cast<uint>(ptrDriveConfiguration->startDeviceNumber()));
            settings.setValue(QStringLiteral("SlotCount"), ptrDriveConfiguration->slotCount());

            //
            // Store only configured disk slots. SlotIndex identifies the
            // actual slot and is independent of the array position.
            //
            const QList<IECFoundryDiskConfiguration>& disks = ptrDriveConfiguration->disks();
            settings.beginWriteArray(QStringLiteral("Disks"), disks.size());

            for (int diskIndex = 0; diskIndex < disks.size(); ++diskIndex)
            {
                settings.setArrayIndex(diskIndex);

                const IECFoundryDiskConfiguration& diskConfiguration = disks[diskIndex];
                const bool saveCurrentDiskOnExit = diskConfiguration.saveCurrentDiskOnExit();

                settings.setValue(QStringLiteral("SlotIndex"), diskConfiguration.slotIndex());
                settings.setValue(QStringLiteral("Enabled"), diskConfiguration.enabled());
                settings.setValue(QStringLiteral("SaveCurrentDiskOnExit"), saveCurrentDiskOnExit);

                //
                // Never retain an old path when persistence is disabled.
                //
                settings.remove(QStringLiteral("ImageFileName"));

                if (saveCurrentDiskOnExit && !diskConfiguration.imageFileName().isEmpty())
                    settings.setValue(QStringLiteral("ImageFileName"), diskConfiguration.imageFileName());
            }

            settings.endArray();

            break;
        }
        }
    }

    settings.endArray();

    return true;
}
