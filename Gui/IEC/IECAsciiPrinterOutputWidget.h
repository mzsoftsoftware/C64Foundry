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
    void setPowered(bool powered);
    void setOutput(const QByteArray& output);

private:
    Ui::IECAsciiPrinterOutputWidget *ui;
};
