#include "IECDeviceRowWidget.h"

#include <QFont>
#include <QLabel>
#include <QMessageBox>

#include "Emulator/EmulatorController.h"
#include "C64/IEC/IECSystemRequest.h"
#include "C64/IEC/IECSystemResponse.h"


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

    m_ptrController = ptrController;

    connect(m_ptrController, &EmulatorController::iecRequestDone,
            this, &IECDeviceRowWidget::onIECRequestDone);

    onControllerSet();
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


bool IECDeviceRowWidget::submitRequest(IECRequest* ptrRequest, RequestPurpose purpose)
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

    submitRequest(new IECSystemGetConfigurationsRequest(),
                  RequestPurpose::LoadSettings);
}


void IECDeviceRowWidget::requestRemove()
{
    if (m_ptrConfiguration.isNull())
        return;

    submitRequest(new IECSystemRemoveDeviceRequest(deviceId()),
                  RequestPurpose::Remove);
}


void IECDeviceRowWidget::requestSetConfiguration(
    const IECDeviceConfiguration& configuration)
{
    m_ptrPendingConfiguration.reset(configuration.clone());

    if (!submitRequest(new IECSystemSetConfigurationRequest(configuration),
                       RequestPurpose::SetConfiguration))
    {
        m_ptrPendingConfiguration.clear();
    }
}


void IECDeviceRowWidget::setDeviceNumberLabel(
    QLabel* ptrLabel, quint8 deviceNumber)
{
    if (ptrLabel == nullptr)
        return;

    QFont font = ptrLabel->font();
    font.setStyleHint(QFont::Monospace);
    font.setFamily(QStringLiteral("Monospace"));
    ptrLabel->setFont(font);

    ptrLabel->setText(
        QStringLiteral("#%1").arg(deviceNumber, 2, 10, QChar('0')));
}


void IECDeviceRowWidget::finishRequest()
{
    m_requestId = 0;
    m_requestPurpose = RequestPurpose::None;
    setBusy(false);
}


void IECDeviceRowWidget::onIECRequestDone(QSharedPointer<IECResponse> response)
{
    if (response.isNull() || m_requestId == 0 ||
        response->requestId() != m_requestId)
    {
        return;
    }

    const IECSystemSetConfigurationResponse* ptrSetResponse =
        dynamic_cast<const IECSystemSetConfigurationResponse*>(response.data());

    if (ptrSetResponse != nullptr && !ptrSetResponse->operationMessage().isEmpty())
    {
        QMessageBox::warning(this, tr("IEC device warning"),
                             ptrSetResponse->operationMessage());
    }

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
        const IECSystemGetConfigurationsResponse* ptrResponse =
            dynamic_cast<const IECSystemGetConfigurationsResponse*>(response.data());

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
            if (ptrConfiguration.isNull() ||
                ptrConfiguration->id() != deviceId())
            {
                continue;
            }

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


void IECDeviceRowWidget::onControllerSet()
{
}
