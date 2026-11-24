#include "IECPrinterOutputDockWidget.h"
#include "ui_IECPrinterOutputDockWidget.h"

#include <QDebug>
#include <QTabWidget>

#include "Emulator/EmulatorController.h"
#include "C64/IEC/IECSystemRequest.h"
#include "C64/IEC/IECSystemResponse.h"
#include "Gui/IEC/IECAsciiPrinterOutputWidget.h"


IECPrinterOutputDockWidget::IECPrinterOutputDockWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECPrinterOutputDockWidget)
{
    ui->setupUi(this);
}

IECPrinterOutputDockWidget::~IECPrinterOutputDockWidget()
{
    clearPrinterTabs();
    delete ui;
}

void IECPrinterOutputDockWidget::setController(EmulatorController* ptrController)
{
    if (m_ptrController == ptrController)
        return;

    if (m_ptrController != nullptr)
        disconnect(m_ptrController, nullptr, this, nullptr);

    m_ptrController = ptrController;
    m_configurationRequestId = 0;
    m_bRefreshPending = false;

    clearPrinterTabs();

    if (m_ptrController == nullptr)
        return;

    connect(m_ptrController,
            &EmulatorController::iecRequestDone,
            this,
            &IECPrinterOutputDockWidget::onRequestDone);

    connect(m_ptrController,
            &EmulatorController::iecSystemEventsChanged,
            this,
            &IECPrinterOutputDockWidget::onIECSystemEventsChanged);

    refreshConfigurations();
}

void IECPrinterOutputDockWidget::refreshConfigurations()
{
    if (m_ptrController == nullptr)
        return;

    if (m_configurationRequestId != 0)
    {
        m_bRefreshPending = true;
        return;
    }

    m_configurationRequestId =
        m_ptrController->request(new IECSystemGetConfigurationsRequest());

    if (m_configurationRequestId == 0)
    {
        qWarning() << "IECPrinterOutputDockWidget: could not submit configuration request";
        m_bRefreshPending = true;
    }
}

void IECPrinterOutputDockWidget::onRequestDone(QSharedPointer<IECResponse> response)
{
    if (response.isNull())
        return;

    if (m_configurationRequestId == 0 ||
        response->requestId() != m_configurationRequestId)
        return;

    m_configurationRequestId = 0;

    bool bRefreshSuccess = false;

    const IECSystemGetConfigurationsResponse* ptrResponse =
        dynamic_cast<const IECSystemGetConfigurationsResponse*>(response.data());

    if (response->success() && ptrResponse != nullptr)
    {
        updatePrinterTabs(ptrResponse->configurations());
        bRefreshSuccess = true;
    }

    if (!bRefreshSuccess)
        qWarning() << "IECPrinterOutputDockWidget: failed to retrieve IEC configurations";

    if (m_bRefreshPending)
    {
        m_bRefreshPending = false;
        refreshConfigurations();
    }
}

void IECPrinterOutputDockWidget::onIECSystemEventsChanged(IECSystemEventList events)
{
    for (const IECSystemEventPtr& ptrEvent : events)
    {
        if (ptrEvent.isNull())
            continue;

        switch (ptrEvent->type)
        {
        case IECSystemEvent::Type::DeviceAdded:
        case IECSystemEvent::Type::DeviceRemoved:
        case IECSystemEvent::Type::ConfigurationChanged:
        case IECSystemEvent::Type::ConfigurationsRestored:
            refreshConfigurations();
            return;

        case IECSystemEvent::Type::Reset:
            // Ein Reset ändert die Geräteliste nicht.
            break;
        }
    }
}

void IECPrinterOutputDockWidget::updatePrinterTabs(
    const QList<QSharedPointer<IECDeviceConfiguration>>& configurations)
{
    QList<QSharedPointer<IECDeviceConfiguration>> sortedConfigurations;
    QHash<IECDeviceConfiguration::DeviceId, QSharedPointer<IECDeviceConfiguration>> newConfigurations;

    for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
            continue;

        newConfigurations.insert(ptrConfiguration->id(), ptrConfiguration);

        if (ptrConfiguration->type() != IECDeviceType::PrinterASCII || !ptrConfiguration->enabled())
            continue;

        qsizetype insertIndex = 0;

        while (insertIndex < sortedConfigurations.size()
               && sortedConfigurations[insertIndex]->deviceNumber() <= ptrConfiguration->deviceNumber())
        {
            ++insertIndex;
        }

        sortedConfigurations.insert(insertIndex, ptrConfiguration);
    }

    // Tabs für entfernte oder nicht mehr unterstützte Drucker entfernen.
    for (IECDeviceConfiguration::DeviceId deviceId : m_printerWidgets.keys())
    {
        const QSharedPointer<IECDeviceConfiguration> ptrConfiguration = newConfigurations.value(deviceId);

        const bool bKeepWidget = !ptrConfiguration.isNull()
                                 && ptrConfiguration->type() == IECDeviceType::PrinterASCII
                                 && ptrConfiguration->enabled();

        if (bKeepWidget)
            continue;

        IECAsciiPrinterOutputWidget* ptrWidget = m_printerWidgets.take(deviceId);
        const int tabIndex = ui->tabWidget_Printers->indexOf(ptrWidget);

        if (tabIndex >= 0)
            ui->tabWidget_Printers->removeTab(tabIndex);

        delete ptrWidget;
    }

    // Vorhandene Tabs aktualisieren und neue Tabs erzeugen.
    for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration : sortedConfigurations)
    {
        const IECDeviceConfiguration::DeviceId deviceId = ptrConfiguration->id();
        IECAsciiPrinterOutputWidget* ptrWidget = m_printerWidgets.value(deviceId, nullptr);

        if (ptrWidget == nullptr)
        {
            ptrWidget = new IECAsciiPrinterOutputWidget(ui->tabWidget_Printers);
            ptrWidget->setController(m_ptrController, deviceId);
            m_printerWidgets.insert(deviceId, ptrWidget);
        }

        const QString printerName = tr("ASCII Printer — IEC #%1")
                                        .arg(ptrConfiguration->deviceNumber());

        ptrWidget->setPrinterName(printerName);

        const int oldIndex = ui->tabWidget_Printers->indexOf(ptrWidget);

        if (oldIndex >= 0)
            ui->tabWidget_Printers->removeTab(oldIndex);

        ui->tabWidget_Printers->insertTab(
            static_cast<int>(ui->tabWidget_Printers->count()),
            ptrWidget,
            tr("IEC #%1").arg(ptrConfiguration->deviceNumber()));
    }
}

void IECPrinterOutputDockWidget::clearPrinterTabs()
{
    while (ui->tabWidget_Printers->count() > 0)
        ui->tabWidget_Printers->removeTab(0);

    qDeleteAll(m_printerWidgets);
    m_printerWidgets.clear();
}
