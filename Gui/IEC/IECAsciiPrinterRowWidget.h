#pragma once

#include "IECDeviceRowWidget.h"
#include "C64/IEC/IECDeviceEvent.h"

namespace Ui {
class IECAsciiPrinterRowWidget;
}


class IECAsciiPrinterRowWidget : public IECDeviceRowWidget
{
    Q_OBJECT

public:
    explicit IECAsciiPrinterRowWidget(QWidget* parent = nullptr);
    virtual ~IECAsciiPrinterRowWidget();

protected:
    void onControllerSet() override;
    void setOperationState(IECOperationStatusWidget::State state, const QString& message = QString()) override;
    void applyConfiguration(const IECDeviceConfiguration& configuration) override;
    void setPowerButtonChecked(bool checked) override;
    void setControlsEnabled(bool enabled) override;
    void openSettingsDialog(const EmulatorResponseIEC* ptrResponse) override;

private slots:
    void on_toolButton_Power_toggled(bool checked);
    void on_toolButton_Settings_clicked();
    void on_toolButton_Remove_clicked();
    void onPrinterOutputChanged(IECDeviceConfiguration::DeviceId deviceId, QQueue<IECPrinterOutputEvent> events);

private:
    void showSettingsDialog(const EmulatorResponseIEC* ptrResponse);
    void triggerActivity(int index = 0);

private:
    Ui::IECAsciiPrinterRowWidget* ui = nullptr;
};
