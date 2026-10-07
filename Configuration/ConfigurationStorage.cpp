#include "ConfigurationStorage.h"

#include <QSettings>
#include <QStringList>

#include "C64Configuration.h"


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
