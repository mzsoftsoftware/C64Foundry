#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Emulator/EmulatorController.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_ptrEmulatorController = new EmulatorController(this);
    ui->dockWidgetContents_EmulatorControl->setController(m_ptrEmulatorController);
}

MainWindow::~MainWindow()
{
    delete ui;
}
