#pragma once

#include "IECDeviceRowWidget.h"
#include "C64/IEC/IECDeviceEvent.h"

namespace Ui {
class IECAsciiPrinterRowWidget;
}

class IECSystemGetConfigurationsResponse;


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
    void openSettingsDialog(const IECSystemGetConfigurationsResponse* ptrResponse) override;

private slots:
    void on_toolButton_Power_toggled(bool checked);
    void on_toolButton_Settings_clicked();
    void on_toolButton_Remove_clicked();
    void onIECDeviceEventsChanged(IECDeviceEventList events);

private:
    void showSettingsDialog(const IECSystemGetConfigurationsResponse* ptrResponse);
    void triggerActivity(int index = 0);

private:
    Ui::IECAsciiPrinterRowWidget* ui = nullptr;
};
