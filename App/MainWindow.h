#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ConfigurationManager;
class EmulatorController;
class VideoController;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void romSetLoaded(bool bLoaded);

private:
    Ui::MainWindow* ui;

    ConfigurationManager* m_ptrConfigurationManager = nullptr;
    EmulatorController* m_ptrEmulatorController = nullptr;
    VideoController* m_ptrVideoController = nullptr;
};
