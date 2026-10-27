#pragma once

#include "IECDeviceRowWidget.h"

namespace Ui {
class IECAsciiPrinterRowWidget;
}


class IECAsciiPrinterRowWidget : public IECDeviceRowWidget
{
    Q_OBJECT

public:
    explicit IECAsciiPrinterRowWidget(QWidget *parent = nullptr);
    virtual ~IECAsciiPrinterRowWidget();

    void setController(EmulatorController* ptrController) override;
    void setOperationState(IECOperationStatusWidget::State state, const QString& message = QString()) override;
    void setConfiguration(const IECDeviceConfiguration& configuration) override;
    void setBusy(bool busy) override;

    //bool isBusy() const                                                             { return m_bBusy; }
    IECDeviceConfiguration::DeviceId deviceId() const override              { return m_deviceId; }

    void triggerActivity(int index = 0);

private slots:
    void on_toolButton_Power_toggled(bool checked);
    void on_toolButton_Settings_clicked();
    void on_toolButton_Remove_clicked();

    void onRequestDone();
    void onIECDeviceChanged();
    void onPrinterOutputChanged();

protected:
    Ui::IECAsciiPrinterRowWidget* ui;

private:
    EmulatorController* m_ptrController = nullptr;
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
    bool m_bBusy = false;
};
