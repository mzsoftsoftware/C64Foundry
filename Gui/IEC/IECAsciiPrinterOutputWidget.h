#ifndef IECASCIIPRINTEROUTPUTWIDGET_H
#define IECASCIIPRINTEROUTPUTWIDGET_H

#include <QWidget>

namespace Ui {
class IECAsciiPrinterOutputWidget;
}

class IECAsciiPrinterOutputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECAsciiPrinterOutputWidget(QWidget *parent = nullptr);
    ~IECAsciiPrinterOutputWidget();

private:
    Ui::IECAsciiPrinterOutputWidget *ui;
};

#endif // IECASCIIPRINTEROUTPUTWIDGET_H
