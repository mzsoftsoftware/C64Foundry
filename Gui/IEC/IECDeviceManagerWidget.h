#pragma once

#include <QWidget>
#include <QHash>
#include <QList>

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

private slots:
    void onAddMenuTriggered(QAction* ptrAction);
    void onRequestDone(QSharedPointer<EmulatorResponse> response);
    void onIECDeviceChanged(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId);
    void on_pushButton_Refresh_clicked();

private:
    void setupAddMenu();
    void updateAddMenu();
    void refreshConfigurations();
    void setConfigurations(const QList<IECDeviceManager::ConfigurationPtr>& configurations, const QHash<IECDeviceConfiguration::DeviceId, bool>& diskImageLoaded);
    void clearDeviceRows();
    void clearDeviceList();
    IECDeviceRowWidget* createDeviceRow(IECDeviceType type);
    QString deviceTypeName(IECDeviceType type) const;

private:
    Ui::IECDeviceManagerWidget* ui = nullptr;
    EmulatorController* m_ptrController = nullptr;

    QHash<IECDeviceConfiguration::DeviceId, IECDeviceRowWidget*> m_deviceRows;
    QList<IECDeviceType> m_availableDeviceTypes;
    QList<IECDeviceType> m_addableDeviceTypes;

    EmulatorRequest::RequestId m_addRequestId = 0;
    EmulatorRequest::RequestId m_configurationRequestId = 0;

    bool m_bRefreshPending = false;
};
