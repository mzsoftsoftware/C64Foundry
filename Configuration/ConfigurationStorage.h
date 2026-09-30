#pragma once

#include <QMap>
#include <QString>

class C64Configuration;


class ConfigurationStorage
{
public:
    ConfigurationStorage() = default;

    bool load(QMap<QString, C64Configuration*>& configurations, QString& activeConfigurationName) const;
    bool save(const QMap<QString, C64Configuration*>& configurations, const QString& activeConfigurationName) const;
};