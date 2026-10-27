#include "IECDeviceRowWidget.h"

#include <QSignalBlocker>

#include "Emulator/EmulatorController.h"


IECDeviceRowWidget::IECDeviceRowWidget(QWidget* parent)
    : QWidget(parent)
{
}

IECDeviceRowWidget::~IECDeviceRowWidget()
{
}

void IECDeviceRowWidget::setController(EmulatorController* ptrController)
{
    Q_ASSERT(ptrController != nullptr);
    Q_ASSERT(m_ptrController == nullptr);

    if (ptrController == nullptr || m_ptrController != nullptr)
        return;

    m_ptrController = ptrController;

    connect(m_ptrController, &EmulatorController::requestDone,
            this, &IECDeviceRowWidget::onRequestDone);

    connect(m_ptrController, &EmulatorController::iecDeviceChanged,
            this, &IECDeviceRowWidget::onIECDeviceChanged);
}

void IECDeviceRowWidget::setConfiguration(const IECDeviceConfiguration& configuration)
{
    m_ptrConfiguration.reset(configuration.clone());

    applyConfiguration(*m_ptrConfiguration);
}

IECDeviceConfiguration::DeviceId IECDeviceRowWidget::deviceId() const
{
    if (m_ptrConfiguration.isNull())
        return 0;

    return m_ptrConfiguration->id();
}

const IECDeviceConfiguration* IECDeviceRowWidget::configuration() const
{
    return m_ptrConfiguration.data();
}

void IECDeviceRowWidget::setBusy(bool busy)
{
    if (m_bBusy == busy)
        return;

    m_bBusy = busy;
    setControlsEnabled(!busy);
}

bool IECDeviceRowWidget::submitRequest(EmulatorRequest* ptrRequest, RequestPurpose purpose)
{
    if (m_ptrController == nullptr || m_bBusy || ptrRequest == nullptr)
    {
        delete ptrRequest;
        return false;
    }

    setBusy(true);
    setOperationState(IECOperationStatusWidget::State::Pending);

    m_requestPurpose = purpose;
    m_requestId = m_ptrController->request(ptrRequest);

    if (m_requestId != 0)
        return true;

    m_requestPurpose = RequestPurpose::None;
    setBusy(false);
    setOperationState(IECOperationStatusWidget::State::Failed,
                      tr("Could not submit IEC device request"));
    return false;
}

void IECDeviceRowWidget::requestPowerChange(bool enabled)
{
    if (m_ptrConfiguration.isNull())
        return;

    QSharedPointer<IECDeviceConfiguration> ptrConfiguration(m_ptrConfiguration->clone());
    ptrConfiguration->setEnabled(enabled);

    setPowerButtonChecked(m_ptrConfiguration->enabled());

    requestSetConfiguration(*ptrConfiguration);
}

void IECDeviceRowWidget::requestSettings()
{
    if (m_ptrConfiguration.isNull())
        return;

    submitRequest(new EmulatorRequestGetIECConfigurations(),
                  RequestPurpose::LoadSettings);
}

void IECDeviceRowWidget::requestRemove()
{
    if (m_ptrConfiguration.isNull())
        return;

    submitRequest(new EmulatorRequestRemoveIECDevice(deviceId()),
                  RequestPurpose::Remove);
}

void IECDeviceRowWidget::requestSetConfiguration(
    const IECDeviceConfiguration& configuration)
{
    m_ptrPendingConfiguration.reset(configuration.clone());

    if (!submitRequest(new EmulatorRequestSetIECConfiguration(configuration),
                       RequestPurpose::SetConfiguration))
    {
        m_ptrPendingConfiguration.clear();
    }
}

void IECDeviceRowWidget::finishRequest()
{
    m_requestId = 0;
    m_requestPurpose = RequestPurpose::None;
    setBusy(false);
}

void IECDeviceRowWidget::onRequestDone(QSharedPointer<EmulatorResponse> response)
{
    if (response.isNull() || m_requestId == 0 || response->id() != m_requestId)
        return;

    const RequestPurpose purpose = m_requestPurpose;

    if (!response->success())
    {
        finishRequest();

        setOperationState(IECOperationStatusWidget::State::Failed,
                          tr("IEC device operation failed"));

        if (!m_ptrConfiguration.isNull())
            setPowerButtonChecked(m_ptrConfiguration->enabled());

        m_ptrPendingConfiguration.clear();
        return;
    }

    switch (purpose)
    {
    case RequestPurpose::SetConfiguration:
        if (!m_ptrPendingConfiguration.isNull())
            setConfiguration(*m_ptrPendingConfiguration);

        m_ptrPendingConfiguration.clear();
        finishRequest();
        setOperationState(IECOperationStatusWidget::State::Success);
        break;

    case RequestPurpose::LoadSettings:
    {
        const EmulatorResponseIEC* ptrResponse =
            dynamic_cast<const EmulatorResponseIEC*>(response.data());

        finishRequest();

        if (ptrResponse == nullptr)
        {
            setOperationState(IECOperationStatusWidget::State::Failed,
                              tr("Invalid IEC configuration response"));
            return;
        }

        for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration :
             ptrResponse->configurations())
        {
            if (ptrConfiguration.isNull() || ptrConfiguration->id() != deviceId())
                continue;

            setConfiguration(*ptrConfiguration);
            break;
        }

        openSettingsDialog(ptrResponse);
        break;
    }

    case RequestPurpose::Remove:
        finishRequest();
        setOperationState(IECOperationStatusWidget::State::Success);
        break;

    case RequestPurpose::None:
        finishRequest();
        break;
    }
}

void IECDeviceRowWidget::onIECDeviceChanged(
    EmulatorIECDeviceEvent::Type type,
    IECDeviceConfiguration::DeviceId changedDeviceId)
{
    Q_UNUSED(type)
    Q_UNUSED(changedDeviceId)

    // Die IECDeviceManagerWidget aktualisiert die Konfigurationen zentral.
    // Hier darf bei ConfigurationChanged kein Einstellungsdialog aufgehen.
}
