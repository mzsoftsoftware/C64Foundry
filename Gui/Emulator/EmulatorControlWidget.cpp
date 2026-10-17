#include "EmulatorControlWidget.h"
#include "ui_EmulatorControlWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

#include "Emulator/EmulatorController.h"
#include "Storage/PRG/C64PRGFile.h"


EmulatorControlWidget::EmulatorControlWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EmulatorControlWidget)
{
    ui->setupUi(this);

    setupSpeedComboBox();
}

EmulatorControlWidget::~EmulatorControlWidget()
{
    delete ui;
}

void EmulatorControlWidget::setController(EmulatorController* ptrController)
{
    m_ptrController = ptrController;

    connect(m_ptrController, &EmulatorController::runningChanged, this, &EmulatorControlWidget::setRunning);
    connect(m_ptrController, &EmulatorController::statisticsChanged, this, &EmulatorControlWidget::onStatisticsChanged);
    connect(m_ptrController, &EmulatorController::programLoaded, this, &EmulatorControlWidget::onProgramLoaded);
}

void EmulatorControlWidget::setRunning(bool bRunning)
{
    ui->pushButton_Start->setEnabled(!bRunning);
    ui->pushButton_Stop->setEnabled(bRunning);
    ui->pushButton_LoadPRG->setEnabled(bRunning);
}

void EmulatorControlWidget::setupSpeedComboBox()
{
    ui->comboBox_Speed->addItem(tr("0,5x"), QVariant::fromValue(Emulator::Speed::Half));
    ui->comboBox_Speed->addItem(tr("1x"), QVariant::fromValue(Emulator::Speed::Normal));
    ui->comboBox_Speed->addItem(tr("2x"), QVariant::fromValue(Emulator::Speed::Double));
    ui->comboBox_Speed->addItem(tr("5x"), QVariant::fromValue(Emulator::Speed::Five));
    ui->comboBox_Speed->addItem(tr("Warp"), QVariant::fromValue(Emulator::Speed::Warp));

    ui->comboBox_Speed->setCurrentIndex(1);
}

void EmulatorControlWidget::on_pushButton_Start_clicked()
{
    if(!m_ptrController)
        return;

    m_ptrController->start();
}
void EmulatorControlWidget::on_pushButton_Reset_clicked()
{
    if(!m_ptrController)
        return;

    m_ptrController->reset();
}
void EmulatorControlWidget::on_pushButton_Stop_clicked()
{
    if(!m_ptrController)
        return;

    m_ptrController->stop();
}


void EmulatorControlWidget::on_comboBox_Speed_currentIndexChanged(int index)
{
    if(!m_ptrController)
        return;

    const QVariant data = ui->comboBox_Speed->itemData(index);
    const Emulator::Speed speed = data.value<Emulator::Speed>();

    m_ptrController->setSpeed(speed);
}

void EmulatorControlWidget::onStatisticsChanged(const quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond)
{
    Q_UNUSED(currentCyclesPerSecond);
    ui->label_Cycles->setText(QString::number(cycles));
    ui->label_CycleRate->setText(QString::number(averageCyclesPerSecond));
}

void EmulatorControlWidget::on_pushButton_LoadPRG_clicked()
{
    if (!m_ptrController)
        return;

    QSettings settings;

    const QString lastDirectory = settings.value(QStringLiteral("FileDialogs/LastPRGDirectory"), QDir::homePath()).toString();

    const QString fileName = QFileDialog::getOpenFileName(this, tr("Load PRG file"), lastDirectory, tr("C64 programs (*.prg);;All files (*)"));

    if (fileName.isEmpty())
        return;

    settings.setValue(QStringLiteral("FileDialogs/LastPRGDirectory"), QFileInfo(fileName).absolutePath());

    C64PRGFile prgFile;

    if (!prgFile.load(fileName))
    {
        QMessageBox::warning(this, tr("Load PRG file"), prgFile.errorString());
        return;
    }

    m_ptrController->loadProgram(prgFile.loadAddress(), prgFile.data());
}
void EmulatorControlWidget::onProgramLoaded(bool bLoaded)
{
    if (!bLoaded)
    {
        QMessageBox::warning(this, tr("Load PRG file"), tr("The PRG file could not be loaded into C64 memory."));
    }
}

