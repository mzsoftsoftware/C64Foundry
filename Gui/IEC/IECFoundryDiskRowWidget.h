#ifndef IECFOUNDRYDISKROWWIDGET_H
#define IECFOUNDRYDISKROWWIDGET_H

#include <QWidget>

namespace Ui {
class IECFoundryDiskRowWidget;
}

class IECFoundryDiskRowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECFoundryDiskRowWidget(QWidget *parent = nullptr);
    ~IECFoundryDiskRowWidget();

private:
    Ui::IECFoundryDiskRowWidget *ui;
};

#endif // IECFOUNDRYDISKROWWIDGET_H
