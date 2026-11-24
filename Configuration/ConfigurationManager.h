#pragma once

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSharedPointer>

#include "C64/IEC/IECRequest.h"
#include "C64/IEC/IECResponse.h"
#include "C64/IEC/IECSystemEvent.h"
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
    bool hasSaveError() const { return m_bSaveError; }

signals:
    void configurationChanged();
    void saveErrorChanged(bool bError);

private slots:
    void onIECSystemEventsChanged(IECSystemEventList events);
    void onIECRequestDone(QSharedPointer<IECResponse> response);

private:
    void requestIECConfigurations();

private:
    QMap<QString, C64Configuration*> m_qmapConfigurations;
    C64Configuration* m_ptrActiveConfiguration = nullptr;

    EmulatorController* m_ptrEmulatorController = nullptr;
    IECRequest::RequestId m_iecConfigurationRequestId = 0;
    bool m_bIECRefreshPending = false;

    bool m_bSaveError = false;
};