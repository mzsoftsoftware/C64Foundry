#pragma once

#include <QWidget>

#include "Emulator/EmulatorTypes.h"

class EmulatorController;

namespace Ui {
class EmulatorControlWidget;
}


class EmulatorControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmulatorControlWidget(QWidget* parent = nullptr);
    ~EmulatorControlWidget();

    // Setter
    void setController(EmulatorController* ptrController);

private slots:
    void on_pushButton_Start_clicked();
    void on_pushButton_Reset_clicked();
    void on_pushButton_Stop_clicked();
    void on_comboBox_Speed_currentIndexChanged(int index);

    void setRunning(bool bRunning);
    void onStatisticsChanged(quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond);

    void on_pushButton_LoadPRG_clicked();
    void onProgramLoaded(bool bLoaded);

private:
    void setupSpeedComboBox();

private:
    Ui::EmulatorControlWidget* ui;

    EmulatorController* m_ptrController = nullptr;
};
