#pragma once

#include <QSharedPointer>
#include <QWidget>

#include "IECOperationStatusWidget.h"
#include "C64/IEC/IECDeviceConfiguration.h"
#include "C64/IEC/IECRequest.h"
#include "C64/IEC/IECResponse.h"

class QLabel;
class EmulatorController;
class IECSystemGetConfigurationsResponse;


class IECDeviceRowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECDeviceRowWidget(QWidget* parent = nullptr);
    virtual ~IECDeviceRowWidget();

    void setController(EmulatorController* ptrController);
    void setConfiguration(const IECDeviceConfiguration& configuration);
    void setBusy(bool busy);

    IECDeviceConfiguration::DeviceId deviceId() const;
    const IECDeviceConfiguration* configuration() const;

protected:
    enum class RequestPurpose
    {
        None,
        SetConfiguration,
        LoadSettings,
        Remove
    };

    virtual void onControllerSet();
    void requestPowerChange(bool enabled);
    void requestSettings();
    void requestRemove();
    void requestSetConfiguration(const IECDeviceConfiguration& configuration);
    void setDeviceNumberLabel(QLabel* ptrLabel, quint8 deviceNumber);

    virtual void setOperationState(IECOperationStatusWidget::State state, const QString& message = QString()) = 0;
    virtual void applyConfiguration(const IECDeviceConfiguration& configuration) = 0;
    virtual void setPowerButtonChecked(bool checked) = 0;
    virtual void setControlsEnabled(bool enabled) = 0;
    virtual void openSettingsDialog(const IECSystemGetConfigurationsResponse* ptrResponse) = 0;

private slots:
    void onIECRequestDone(QSharedPointer<IECResponse> response);

protected:
    EmulatorController* m_ptrController = nullptr;
    bool m_bBusy = false;

private:
    bool submitRequest(IECRequest* ptrRequest, RequestPurpose purpose);
    void finishRequest();

    QSharedPointer<IECDeviceConfiguration> m_ptrConfiguration;
    QSharedPointer<IECDeviceConfiguration> m_ptrPendingConfiguration;

    IECRequest::RequestId m_requestId = 0;
    RequestPurpose m_requestPurpose = RequestPurpose::None;
};
