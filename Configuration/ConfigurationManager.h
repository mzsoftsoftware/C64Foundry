#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSharedPointer>

#include "Emulator/EmulatorRequest.h"
#include "Emulator/EmulatorResponse.h"
#include "Emulator/EmulatorIECDeviceEvent.h"

#include "C64/IEC/IECDeviceConfiguration.h"

class C64Configuration;
class EmulatorController;

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
    void startIECSynchronization(EmulatorController* ptrController);

    // Getter
    QStringList configurationNames() const                                  { return m_qmapConfigurations.keys(); }
    const C64Configuration* configuration(const QString& qstrName) const    { return m_qmapConfigurations.value(qstrName, nullptr); }
    const C64Configuration* activeConfiguration() const                     { return m_ptrActiveConfiguration; }

signals:
    void configurationChanged();

private slots:
    void onIECDeviceChanged(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId);
    void onIECRequestDone(QSharedPointer<EmulatorResponse> response);

private:
    void requestIECConfigurations();

private:
    QMap<QString, C64Configuration*> m_qmapConfigurations;
    C64Configuration* m_ptrActiveConfiguration = nullptr;

    EmulatorController* m_ptrEmulatorController = nullptr;
    EmulatorRequest::RequestId m_iecConfigurationRequestId = 0;
    bool m_bIECRefreshPending = false;
};