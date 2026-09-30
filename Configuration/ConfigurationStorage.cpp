#include "ConfigurationStorage.h"

#include <QSettings>

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

        settings.beginGroup(QStringLiteral("ROMSet"));
        ptrConfiguration->romSet.basicROMFileName = settings.value(QStringLiteral("Basic")).toString();
        ptrConfiguration->romSet.kernalROMFileName = settings.value(QStringLiteral("Kernal")).toString();
        ptrConfiguration->romSet.characterROMFileName = settings.value(QStringLiteral("Character")).toString();
        settings.endGroup();

        settings.endGroup();

        configurations.insert(configurationName, ptrConfiguration);
    }
    settings.endGroup();

    return true;
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

        settings.beginGroup(QStringLiteral("ROMSet"));
        settings.setValue(QStringLiteral("Basic"), ptrConfiguration->romSet.basicROMFileName);
        settings.setValue(QStringLiteral("Kernal"), ptrConfiguration->romSet.kernalROMFileName);
        settings.setValue(QStringLiteral("Character"), ptrConfiguration->romSet.characterROMFileName);
        settings.endGroup();

        settings.endGroup();
    }
    settings.endGroup();
    settings.sync();

    return settings.status() == QSettings::NoError;
}
