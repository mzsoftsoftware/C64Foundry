#pragma once

#include <QWidget>

#include "C64/IEC/IECDeviceConfiguration.h"
#include "IECOperationStatusWidget.h"

namespace Ui {
class IECDeviceRowWidget;
}


class IECDeviceRowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECDeviceRowWidget(QWidget *parent = nullptr);
    virtual ~IECDeviceRowWidget();

    void setOperationState(IECOperationStatusWidget::State state, const QString& message = QString());
    void setConfiguration(const IECDeviceConfiguration& configuration);

    void setBusy(bool busy);
    bool isBusy() const                                                             { return m_bBusy; }
    IECDeviceConfiguration::DeviceId deviceId() const                               { return m_deviceId; }

signals:
    void enabledChanged(IECDeviceConfiguration::DeviceId deviceId, bool enabled);
    void settingsRequested(IECDeviceConfiguration::DeviceId deviceId);
    void removeRequested(IECDeviceConfiguration::DeviceId deviceId);

private slots:
    void on_toolButton_Power_toggled(bool checked);
    void on_pushButton_Settings_clicked();
    void on_pushButton_Remove_clicked();

protected:
    Ui::IECDeviceRowWidget* ui;

private:
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
    bool m_bBusy = false;
};
