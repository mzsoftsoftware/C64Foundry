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
    ui->pushButton_Power->setStyleSheet(QStringLiteral("QPushButton:checked { background-color: #248a39; color: white; }"));
    setState(Emulator::State::PoweredOff);
}

EmulatorControlWidget::~EmulatorControlWidget()
{
    delete ui;
}

void EmulatorControlWidget::setController(EmulatorController* ptrController)
{
    m_ptrController = ptrController;

    connect(m_ptrController, &EmulatorController::stateChanged, this, &EmulatorControlWidget::setState);
    connect(m_ptrController, &EmulatorController::statisticsChanged, this, &EmulatorControlWidget::onStatisticsChanged);
    connect(m_ptrController, &EmulatorController::programLoaded, this, &EmulatorControlWidget::onProgramLoaded);
}

void EmulatorControlWidget::setState(Emulator::State state)
{
    const bool bPoweredOn = state != Emulator::State::PoweredOff;
    const bool bPaused = state == Emulator::State::Paused;

    ui->pushButton_Power->setChecked(bPoweredOn);
    ui->pushButton_Pause->setChecked(bPaused);

    ui->pushButton_Pause->setEnabled(bPoweredOn);
    ui->pushButton_SoftReset->setEnabled(bPoweredOn);
    ui->pushButton_HardReset->setEnabled(bPoweredOn);

    ui->pushButton_LoadPRG->setEnabled(bPoweredOn);
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

void EmulatorControlWidget::on_pushButton_Power_clicked()
{
    const bool bPowerOn = ui->pushButton_Power->isChecked();

    ui->pushButton_Power->setChecked(!bPowerOn);

    if (!m_ptrController)
        return;

    if (bPowerOn)
        m_ptrController->powerOn();
    else
        m_ptrController->powerOff();
}

void EmulatorControlWidget::on_pushButton_Pause_clicked()
{
    const bool bPaused = ui->pushButton_Pause->isChecked();

    ui->pushButton_Pause->setChecked(!bPaused);

    if (!m_ptrController)
        return;

    m_ptrController->pause(bPaused);
}

void EmulatorControlWidget::on_pushButton_SoftReset_clicked()
{
    if (!m_ptrController)
        return;

    m_ptrController->softReset();
}

void EmulatorControlWidget::on_pushButton_HardReset_clicked()
{
    if (!m_ptrController)
        return;

    m_ptrController->hardReset();
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

