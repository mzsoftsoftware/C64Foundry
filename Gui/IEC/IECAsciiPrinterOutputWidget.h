#pragma once

#include <QWidget>

namespace Ui {
class IECAsciiPrinterOutputWidget;
}


class IECAsciiPrinterOutputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECAsciiPrinterOutputWidget(QWidget *parent = nullptr);
    virtual ~IECAsciiPrinterOutputWidget();

    void setPrinterName(const QString& name);
    void setOutput(const QByteArray& output);

private slots:
    void on_toolButton_Save_clicked();
    void on_toolButton_TearOff_clicked();

private:
    Ui::IECAsciiPrinterOutputWidget *ui;
};
