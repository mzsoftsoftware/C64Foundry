#ifndef IECPRINTEROUTPUTDOCKWIDGET_H
#define IECPRINTEROUTPUTDOCKWIDGET_H

#include <QWidget>

namespace Ui {
class IECPrinterOutputDockWidget;
}

class IECPrinterOutputDockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECPrinterOutputDockWidget(QWidget *parent = nullptr);
    ~IECPrinterOutputDockWidget();

private:
    Ui::IECPrinterOutputDockWidget *ui;
};

#endif // IECPRINTEROUTPUTDOCKWIDGET_H
