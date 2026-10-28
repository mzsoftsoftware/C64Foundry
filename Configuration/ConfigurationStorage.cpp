#include "ConfigurationStorage.h"

#include <QSettings>
#include <QStringList>

#include "C64Configuration.h"
#include "C64/IEC/IECAsciiPrinter.h"
#include "C64/IEC/IECAsciiPrinterConfiguration.h"
#include "C64/IEC/IECFoundryDisk.h"
#include "C64/IEC/IECFoundryDiskConfiguration.h"


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
        saveIECDevices(settings, ptrConfiguration->iecDevices);
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
    QList<IECDeviceConfiguration::DeviceId> usedIds;
    QList<quint8> usedDeviceNumbers;

    const int deviceCount = settings.beginReadArray(QStringLiteral("Devices"));

    for (int index = 0; index < deviceCount; ++index)
    {
        settings.setArrayIndex(index);

        bool bIdValid = false;
        bool bDeviceNumberValid = false;

        const IECDeviceConfiguration::DeviceId id = settings.value(QStringLiteral("Id")).toULongLong(&bIdValid);
        const uint deviceNumber = settings.value(QStringLiteral("DeviceNumber")).toUInt(&bDeviceNumberValid);
        const QString typeName = settings.value(QStringLiteral("Type")).toString();

        if (!bIdValid || id == 0 || !bDeviceNumberValid || deviceNumber > 255 || usedIds.contains(id) || usedDeviceNumbers.contains(static_cast<quint8>(deviceNumber)))
        {
            settings.endArray();
            return false;
        }

        QSharedPointer<IECDeviceConfiguration> ptrConfiguration;

        if (typeName == QStringLiteral("PrinterASCII"))
        {
            if (!IECAsciiPrinter::supportedDeviceNumbers().contains(static_cast<quint8>(deviceNumber)))
            {
                settings.endArray();
                return false;
            }

            bool bLineWidthValid = false;
            const int lineWidth = settings.value(QStringLiteral("LineWidth"), 80).toInt(&bLineWidthValid);

            if (!bLineWidthValid || lineWidth < 1)
            {
                settings.endArray();
                return false;
            }

            IECAsciiPrinterConfiguration* ptrPrinterConfiguration = new IECAsciiPrinterConfiguration();

            ptrPrinterConfiguration->setLineWidth(lineWidth);
            ptrPrinterConfiguration->setAutoWrap(settings.value(QStringLiteral("AutoWrap"), true).toBool());

            ptrConfiguration.reset(ptrPrinterConfiguration);
        }
        else if (typeName == QStringLiteral("FoundryDisk"))
        {
            if (!IECFoundryDisk::supportedDeviceNumbers().contains(static_cast<quint8>(deviceNumber)))
            {
                settings.endArray();
                return false;
            }

            IECFoundryDiskConfiguration* ptrDiskConfiguration = new IECFoundryDiskConfiguration();
            const bool saveCurrentDiskOnExit = settings.value(QStringLiteral("SaveCurrentDiskOnExit"), false).toBool();
            ptrDiskConfiguration->setSaveCurrentDiskOnExit(saveCurrentDiskOnExit);
            if (saveCurrentDiskOnExit)
            {
                ptrDiskConfiguration->setImageFileName(settings.value(QStringLiteral("ImageFileName")).toString());
            }
            ptrConfiguration.reset(ptrDiskConfiguration);
        }
        else
        {
            settings.endArray();
            return false;
        }

        ptrConfiguration->setId(id);
        ptrConfiguration->setDeviceNumber(static_cast<quint8>(deviceNumber));
        ptrConfiguration->setEnabled(settings.value(QStringLiteral("Enabled"), false).toBool());

        loadedDevices.append(ptrConfiguration);
        usedIds.append(id);
        usedDeviceNumbers.append(static_cast<quint8>(deviceNumber));
    }

    settings.endArray();

    devices = loadedDevices;

    return true;
}

void ConfigurationStorage::saveIECDevices(QSettings& settings, const QList<QSharedPointer<IECDeviceConfiguration>>& devices) const
{
    settings.beginWriteArray(QStringLiteral("Devices"), devices.size());

    for (int index = 0; index < devices.size(); ++index)
    {
        settings.setArrayIndex(index);

        const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration = devices[index];

        if (ptrConfiguration.isNull())
            continue;

        settings.setValue(QStringLiteral("Id"), static_cast<qulonglong>(ptrConfiguration->id()));
        settings.setValue(QStringLiteral("DeviceNumber"), static_cast<uint>(ptrConfiguration->deviceNumber()));
        settings.setValue(QStringLiteral("Enabled"), ptrConfiguration->enabled());

        switch (ptrConfiguration->type())
        {
        case IECDeviceType::PrinterASCII:
        {
            const IECAsciiPrinterConfiguration* ptrPrinterConfiguration = dynamic_cast<const IECAsciiPrinterConfiguration*>(ptrConfiguration.data());

            if (ptrPrinterConfiguration == nullptr)
                break;

            settings.setValue(QStringLiteral("Type"), QStringLiteral("PrinterASCII"));
            settings.setValue(QStringLiteral("LineWidth"), ptrPrinterConfiguration->lineWidth());
            settings.setValue(QStringLiteral("AutoWrap"), ptrPrinterConfiguration->autoWrap());

            break;
        }
        case IECDeviceType::FoundryDisk:
        {
            const IECFoundryDiskConfiguration* ptrDiskConfiguration = dynamic_cast<const IECFoundryDiskConfiguration*>(ptrConfiguration.data());

            if (ptrDiskConfiguration == nullptr)
                break;

            settings.setValue(QStringLiteral("Type"), QStringLiteral("FoundryDisk"));
            const bool saveCurrentDiskOnExit = ptrDiskConfiguration->saveCurrentDiskOnExit();
            settings.setValue(QStringLiteral("SaveCurrentDiskOnExit"), saveCurrentDiskOnExit);
            // Alte Image-Pfade bei deaktivierter Option ausdrücklich entfernen.
            settings.remove(QStringLiteral("ImageFileName"));
            if (saveCurrentDiskOnExit && !ptrDiskConfiguration->imageFileName().isEmpty())
            {
                settings.setValue(QStringLiteral("ImageFileName"), ptrDiskConfiguration->imageFileName());
            }
            break;
        }
        }
    }

    settings.endArray();
}
