#include "IECPrinterOutputDockWidget.h"
#include "ui_IECPrinterOutputDockWidget.h"

#include <QDebug>
#include <QTabWidget>

#include "Emulator/EmulatorController.h"
#include "C64/IEC/IECDeviceConfiguration.h"
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
    m_configurations.clear();
    m_printerOutputs.clear();
    m_printerRevisions.clear();

    if (m_ptrController == nullptr)
        return;

    connect(m_ptrController, &EmulatorController::requestDone, this, &IECPrinterOutputDockWidget::onRequestDone);
    connect(m_ptrController, &EmulatorController::iecDeviceChanged, this, &IECPrinterOutputDockWidget::onIECDeviceChanged);
    connect(m_ptrController, &EmulatorController::printerOutputChanged, this, &IECPrinterOutputDockWidget::onPrinterOutputChanged);

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
        m_ptrController->request(new EmulatorRequestGetIECConfigurations());

    if (m_configurationRequestId == 0)
    {
        qWarning() << "IECPrinterOutputDockWidget: could not submit configuration request";
        return;
    }
}

void IECPrinterOutputDockWidget::onRequestDone(QSharedPointer<EmulatorResponse> response)
{
    if (response.isNull())
        return;

    if (m_configurationRequestId == 0 || response->id() != m_configurationRequestId)
        return;

    m_configurationRequestId = 0;

    bool bRefreshSuccess = false;

    if (response->type() == EmulatorRequest::Type::GetIECConfigurations && response->success())
    {
        const EmulatorResponseIEC* ptrResponse =
            dynamic_cast<const EmulatorResponseIEC*>(response.data());

        if (ptrResponse != nullptr)
        {
            updatePrinterTabs(ptrResponse->configurations());
            bRefreshSuccess = true;
        }
    }

    if (!bRefreshSuccess)
        qWarning() << "IECPrinterOutputDockWidget: failed to retrieve IEC configurations";

    if (m_bRefreshPending)
    {
        m_bRefreshPending = false;
        refreshConfigurations();
    }
}

void IECPrinterOutputDockWidget::onIECDeviceChanged(
    EmulatorIECDeviceEvent::Type type,
    IECDeviceConfiguration::DeviceId deviceId)
{
    Q_UNUSED(deviceId)

    switch (type)
    {
    case EmulatorIECDeviceEvent::Type::Added:
    case EmulatorIECDeviceEvent::Type::Removed:
    case EmulatorIECDeviceEvent::Type::ConfigurationChanged:
    case EmulatorIECDeviceEvent::Type::Restored:
        refreshConfigurations();
        break;

    case EmulatorIECDeviceEvent::Type::Reset:
        // Ein Reset ändert die Geräteliste nicht.
        break;
    }
}

void IECPrinterOutputDockWidget::onPrinterOutputChanged(IECDeviceConfiguration::DeviceId deviceId, quint64 revision, QByteArray output)
{
    if (revision < m_printerRevisions.value(deviceId, 0))
        return;

    m_printerRevisions.insert(deviceId, revision);
    m_printerOutputs.insert(deviceId, output);
}

void IECPrinterOutputDockWidget::updatePrinterTabs(
    const QList<IECDeviceManager::ConfigurationPtr>& configurations)
{
    QList<IECDeviceManager::ConfigurationPtr> sortedConfigurations;
    QHash<IECDeviceConfiguration::DeviceId, IECDeviceManager::ConfigurationPtr> newConfigurations;

    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
            continue;

        const IECDeviceManager::ConfigurationPtr ptrOldConfiguration = m_configurations.value(ptrConfiguration->id());
        if (!ptrOldConfiguration.isNull()
            && !ptrOldConfiguration->enabled()
            && ptrConfiguration->enabled())
        {
            m_printerOutputs.remove(ptrConfiguration->id());
            m_printerRevisions.remove(ptrConfiguration->id());
        }
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
        const IECDeviceManager::ConfigurationPtr ptrConfiguration = newConfigurations.value(deviceId);
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

        // Ausgabe nur bei tatsächlich entferntem Gerät oder geändertem Gerätetyp löschen.
        if (ptrConfiguration.isNull()
            || ptrConfiguration->type() != IECDeviceType::PrinterASCII)
        {
            m_printerOutputs.remove(deviceId);
            m_printerRevisions.remove(deviceId);
        }
    }

    // Vorhandene Tabs aktualisieren und neue Tabs erzeugen.
    for (const IECDeviceManager::ConfigurationPtr& ptrConfiguration : sortedConfigurations)
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
        ptrWidget->setOutput(m_printerOutputs.value(deviceId));

        // Bestehenden Tab entfernen und an der sortierten Position wieder einsetzen.
        const int oldIndex = ui->tabWidget_Printers->indexOf(ptrWidget);

        if (oldIndex >= 0)
            ui->tabWidget_Printers->removeTab(oldIndex);

        ui->tabWidget_Printers->insertTab(
            static_cast<int>(ui->tabWidget_Printers->count()),
            ptrWidget,
            tr("IEC #%1").arg(ptrConfiguration->deviceNumber()));
    }

    m_configurations = newConfigurations;
}

void IECPrinterOutputDockWidget::clearPrinterTabs()
{
    while (ui->tabWidget_Printers->count() > 0)
        ui->tabWidget_Printers->removeTab(0);

    qDeleteAll(m_printerWidgets);
    m_printerWidgets.clear();
}
