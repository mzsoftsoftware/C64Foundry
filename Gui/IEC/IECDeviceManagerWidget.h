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

    void on_pushButton_Refresh_clicked();

private:
    void setupAddMenu();

    void clearDeviceRows();
    void refreshConfigurations();

private:
    Ui::IECDeviceManagerWidget* ui;
    QHash<IECDeviceConfiguration::DeviceId, IECDeviceRowWidget*> m_deviceRows;

    EmulatorController* m_ptrController = nullptr;
    EmulatorRequest::RequestId m_configurationRequestId = 0;
    bool m_bRefreshPending = false;

    EmulatorRequest::RequestId m_addRequestId = 0;
};
