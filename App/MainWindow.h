#pragma once

#include <QMainWindow>
#include <QSharedPointer>

#include "Emulator/EmulatorRequest.h"
#include "Emulator/EmulatorResponse.h"

#include "C64/IEC/IECRequest.h"
#include "C64/IEC/IECResponse.h"
#include "C64/VideoFrameSource.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ConfigurationManager;
class EmulatorController;
class KeyboardController;
class VideoController;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    bool initialize();

private slots:
    void onIECConfigurationsRestored(QSharedPointer<IECResponse> response);
    void onVideoFrameSourceReady(VideoFrameSource* ptrSource);
    void romSetLoaded(bool bLoaded);
    void onConfigurationSaveErrorChanged(bool bError);

protected:
    void closeEvent(QCloseEvent* ptrEvent) override;    

private:
    void setupDockWidgetsMenu();

    void saveWindowLayout();
    void restoreWindowLayout();
    void resetWindowLayout();

    bool validateConfiguration();

private:
    Ui::MainWindow* ui;

    ConfigurationManager* m_ptrConfigurationManager = nullptr;
    EmulatorController* m_ptrEmulatorController = nullptr;

    KeyboardController* m_ptrKeyboardController = nullptr;

    VideoController* m_ptrVideoController = nullptr;

    IECRequest::RequestId m_restoreIECRequestId = 0;
};
