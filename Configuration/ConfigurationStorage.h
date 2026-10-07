#pragma once

#include <QMap>
#include <QString>

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
};
