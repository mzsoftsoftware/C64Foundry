#pragma once

#include <QWidget>
#include <QList>
#include <QHash>

#include "Emulator/EmulatorRequest.h"
#include "Emulator/EmulatorResponse.h"
#include "Emulator/EmulatorIECDeviceEvent.h"

#include "C64/IEC/IECDeviceManager.h"

class QAction;
class EmulatorController;
class IECDeviceRowWidget;


namespace Ui {
class IECDeviceManagerWidget;
}


class IECDeviceManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECDeviceManagerWidget(QWidget* parent = nullptr);
    virtual ~IECDeviceManagerWidget();

    void setController(EmulatorController* ptrController);
    void setConfigurations(const QList<IECDeviceManager::ConfigurationPtr>& configurations);

signals:
    void addDeviceRequested();

    void deviceEnabledChanged(IECDeviceConfiguration::DeviceId deviceId, bool enabled);
    void deviceSettingsRequested(IECDeviceConfiguration::DeviceId deviceId);
    void deviceRemoveRequested(IECDeviceConfiguration::DeviceId deviceId);

private slots:
    void onAddMenuTriggered(QAction* ptrAction);

    void onRequestDone(QSharedPointer<EmulatorResponse> response);
    void onIECDeviceChanged(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId);
    void onDeviceEnabledChanged(IECDeviceConfiguration::DeviceId deviceId, bool enabled);
    void onDeviceRemoveRequested(IECDeviceConfiguration::DeviceId deviceId);

    void onDeviceSettingsRequested(IECDeviceConfiguration::DeviceId deviceId);
    void on_pushButton_Refresh_clicked();

private:
    QString deviceTypeName(IECDeviceType type) const;
    void setupAddMenu();
    void updateAddMenu();

    void clearDeviceRows();
    void refreshConfigurations();

    void setBusy(bool busy);
    void updateBusyState();

    QList<quint8> availableDeviceNumbers(IECDeviceType type, IECDeviceConfiguration::DeviceId excludedId) const;

private:
    Ui::IECDeviceManagerWidget* ui;
    QHash<IECDeviceConfiguration::DeviceId, IECDeviceRowWidget*> m_deviceRows;

    EmulatorController* m_ptrController = nullptr;
    EmulatorRequest::RequestId m_configurationRequestId = 0;
    bool m_bRefreshPending = false;
    bool m_bBusy = false;

    QList<IECDeviceType> m_availableDeviceTypes;
    QList<IECDeviceType> m_addableDeviceTypes;

    EmulatorRequest::RequestId m_addRequestId = 0;
    EmulatorRequest::RequestId m_removeRequestId = 0;
    QHash<IECDeviceConfiguration::DeviceId, IECDeviceManager::ConfigurationPtr> m_configurations;
    QHash<EmulatorRequest::RequestId, IECDeviceConfiguration::DeviceId> m_setConfigurationRequests;
    QHash<IECDeviceConfiguration::DeviceId, bool> m_confirmedSetRequests;
    QHash<IECDeviceType, QList<quint8>> m_supportedDeviceNumbers;
};
