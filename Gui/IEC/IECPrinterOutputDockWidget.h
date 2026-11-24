#pragma once

#include <QWidget>
#include <QHash>
#include <QByteArray>

#include "C64/IEC/IECRequest.h"
#include "C64/IEC/IECResponse.h"
#include "C64/IEC/IECSystemEvent.h"
#include "C64/IEC/IECDeviceConfiguration.h"

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
    void onRequestDone(QSharedPointer<IECResponse> response);
    void onIECSystemEventsChanged(IECSystemEventList events);

private:
    void refreshConfigurations();
    void updatePrinterTabs(const QList<QSharedPointer<IECDeviceConfiguration>>& configurations);
    void clearPrinterTabs();

private:
    Ui::IECPrinterOutputDockWidget* ui;

    EmulatorController* m_ptrController = nullptr;
    IECRequest::RequestId m_configurationRequestId = 0;
    bool m_bRefreshPending = false;

    QHash<IECDeviceConfiguration::DeviceId, IECAsciiPrinterOutputWidget*> m_printerWidgets;
};
