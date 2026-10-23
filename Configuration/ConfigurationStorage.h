#pragma once

#include <QMap>
#include <QList>
#include <QSharedPointer>
#include <QString>

#include "C64/IEC/IECDeviceConfiguration.h"

class QSettings;
class C64Configuration;
class C64KeyboardConfiguration;


class ConfigurationStorage
{
public:
    ConfigurationStorage() = default;

    bool load(QMap<QString, C64Configuration*>& configurations, QString& activeConfigurationName) const;
    bool save(const QMap<QString, C64Configuration*>& configurations, const QString& activeConfigurationName) const;

private:
    bool loadKeyboard(QSettings& settings, C64KeyboardConfiguration& keyboard) const;
    void saveKeyboard(QSettings& settings, const C64KeyboardConfiguration& keyboard) const;

    bool loadIECDevices(QSettings& settings, QList<QSharedPointer<IECDeviceConfiguration>>& devices) const;
    void saveIECDevices(QSettings& settings, const QList<QSharedPointer<IECDeviceConfiguration>>& devices) const;
};
