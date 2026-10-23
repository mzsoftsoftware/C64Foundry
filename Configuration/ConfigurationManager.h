#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSharedPointer>

#include "C64/IEC/IECDeviceConfiguration.h"

class C64Configuration;


class ConfigurationManager : public QObject
{
    Q_OBJECT

public:
    explicit ConfigurationManager(QObject* parent);
    virtual ~ConfigurationManager();

    // Initialization
    bool initialize();
    bool save();
    bool setIECConfigurations(const QList<QSharedPointer<IECDeviceConfiguration>>& configurations);

    // Getter
    QStringList configurationNames() const                                  { return m_qmapConfigurations.keys(); }
    const C64Configuration* configuration(const QString& qstrName) const    { return m_qmapConfigurations.value(qstrName, nullptr); }
    const C64Configuration* activeConfiguration() const                     { return m_ptrActiveConfiguration; }

signals:
    void configurationChanged();

private:
    QMap<QString, C64Configuration*> m_qmapConfigurations;
    C64Configuration* m_ptrActiveConfiguration = nullptr;
};