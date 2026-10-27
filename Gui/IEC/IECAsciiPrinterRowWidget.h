#pragma once

#include <QWidget>

#include "C64/IEC/IECDeviceConfiguration.h"
#include "IECOperationStatusWidget.h"

namespace Ui {
class IECAsciiPrinterRowWidget;
}


class IECAsciiPrinterRowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECAsciiPrinterRowWidget(QWidget *parent = nullptr);
    virtual ~IECAsciiPrinterRowWidget();

    void setOperationState(IECOperationStatusWidget::State state, const QString& message = QString());
    void setConfiguration(const IECDeviceConfiguration& configuration);

    void setBusy(bool busy);
    bool isBusy() const                                                             { return m_bBusy; }
    IECDeviceConfiguration::DeviceId deviceId() const                               { return m_deviceId; }

    void triggerActivity(int index = 0);

signals:
    void enabledChanged(IECDeviceConfiguration::DeviceId deviceId, bool enabled);
    void settingsRequested(IECDeviceConfiguration::DeviceId deviceId);
    void removeRequested(IECDeviceConfiguration::DeviceId deviceId);

private slots:
    void on_toolButton_Power_toggled(bool checked);
    void on_toolButton_Settings_clicked();
    void on_toolButton_Remove_clicked();

protected:
    Ui::IECAsciiPrinterRowWidget* ui;

private:
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
    bool m_bBusy = false;
};
