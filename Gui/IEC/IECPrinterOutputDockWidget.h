#pragma once

#include <QWidget>
#include <QHash>
#include <QByteArray>

#include "Emulator/EmulatorRequest.h"
#include "Emulator/EmulatorResponse.h"
#include "Emulator/EmulatorTypes.h"
#include "Emulator/EmulatorIECDeviceEvent.h"
#include "C64/IEC/IECDeviceManager.h"

    class EmulatorController;
class IECAsciiPrinterOutputWidget;

namespace Ui {
class IECPrinterOutputDockWidget;
}

class IECPrinterOutputDockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECPrinterOutputDockWidget(QWidget* parent = nullptr);
    ~IECPrinterOutputDockWidget() override;

    void setController(EmulatorController* ptrController);

private slots:
    void onRequestDone(QSharedPointer<EmulatorResponse> response);
    void onIECDeviceChanged(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId);

private:
    void refreshConfigurations();
    void updatePrinterTabs(const QList<IECDeviceManager::ConfigurationPtr>& configurations);
    void clearPrinterTabs();

private:
    Ui::IECPrinterOutputDockWidget* ui;

    EmulatorController* m_ptrController = nullptr;
    EmulatorRequest::RequestId m_configurationRequestId = 0;
    bool m_bRefreshPending = false;

    QHash<IECDeviceConfiguration::DeviceId, IECDeviceManager::ConfigurationPtr> m_configurations;
    QHash<IECDeviceConfiguration::DeviceId, IECAsciiPrinterOutputWidget*> m_printerWidgets;
};
