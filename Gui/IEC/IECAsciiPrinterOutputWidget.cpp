#include "IECAsciiPrinterOutputWidget.h"
#include "ui_IECAsciiPrinterOutputWidget.h"


#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QMessageBox>
#include <QFileDialog>

#include "Emulator/EmulatorController.h"


IECAsciiPrinterOutputWidget::IECAsciiPrinterOutputWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECAsciiPrinterOutputWidget)
{
    ui->setupUi(this);

    ui->plainTextEdit_Paper->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->plainTextEdit_Paper->setStyleSheet("QPlainTextEdit { background-color: #D8D8D8; color: black; }");
}

IECAsciiPrinterOutputWidget::~IECAsciiPrinterOutputWidget()
{
    delete ui;
}

void IECAsciiPrinterOutputWidget::setController(EmulatorController* ptrController, IECDeviceConfiguration::DeviceId deviceId)
{
    if (m_ptrController == ptrController && m_deviceId == deviceId)
        return;

    if (m_ptrController != nullptr)
        disconnect(m_ptrController, nullptr, this, nullptr);

    m_ptrController = ptrController;
    m_deviceId = deviceId;
    m_outputRevision = 0;

    if (m_ptrController == nullptr)
        return;

    connect(m_ptrController, &EmulatorController::printerOutputChanged, this, &IECAsciiPrinterOutputWidget::onPrinterOutputChanged);
}

void IECAsciiPrinterOutputWidget::setPrinterName(const QString& name)
{
    ui->label_PrinterName->setText(name);
}

void IECAsciiPrinterOutputWidget::on_toolButton_Save_clicked()
{
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Druckerausgabe speichern"),
        QStringLiteral("printer-output.txt"),
        tr("Textdateien (*.txt);;Alle Dateien (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("Speichern fehlgeschlagen"),
                              tr("Die Datei konnte nicht geöffnet werden:\n%1").arg(file.errorString()));
        return;
    }

    const QByteArray output = ui->plainTextEdit_Paper->toPlainText().toLatin1();

    if (file.write(output) != output.size() || !file.flush())
    {
        QMessageBox::critical(this, tr("Speichern fehlgeschlagen"),
                              tr("Die Druckerausgabe konnte nicht vollständig geschrieben werden:\n%1").arg(file.errorString()));
        return;
    }

    file.close();

    if (ui->checkBox_AutoTearOffOnSave->isChecked())
        on_toolButton_TearOff_clicked();
}

void IECAsciiPrinterOutputWidget::on_toolButton_TearOff_clicked()
{
    if (m_ptrController == nullptr || m_deviceId == 0)
        return;

    m_ptrController->request(new EmulatorRequestTearOffIECPrinter(m_deviceId));
}

void IECAsciiPrinterOutputWidget::onPrinterOutputChanged(IECDeviceConfiguration::DeviceId deviceId, quint64 revision, QByteArray output)
{
    if (deviceId != m_deviceId)
        return;

    if (revision < m_outputRevision)
        return;

    m_outputRevision = revision;

    ui->plainTextEdit_Paper->setPlainText(QString::fromLatin1(output));

    QTextDocument* document = ui->plainTextEdit_Paper->document();
    QTextBlock block = document->begin();

    int row = 0;

    while (block.isValid())
    {
        QTextBlock nextBlock = block.next();

        // Der letzte leere Block nach einem abschließenden Zeilenumbruch
        // ist nur der Abschlussblock des Texteditors.
        bool isTrailingEmptyBlock =
            block.text().isEmpty() && !nextBlock.isValid() && !output.isEmpty()
            && (output.endsWith('\n') || output.endsWith('\r'));

        QTextCursor cursor(block);
        QTextBlockFormat blockFormat = block.blockFormat();
        QTextCharFormat charFormat;

        charFormat.setForeground(QColor(Qt::black));

        if (!isTrailingEmptyBlock)
        {
            if ((row % 2) == 0)
            {
                blockFormat.setBackground(QColor(216, 216, 216));
            }
            else
            {
                blockFormat.setBackground(QColor(187, 214, 178));
            }

            blockFormat.setLineHeight(95, QTextBlockFormat::ProportionalHeight);
            ++row;
        }
        else
        {
            blockFormat.clearBackground();
        }

        cursor.setBlockFormat(blockFormat);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.mergeCharFormat(charFormat);

        block = nextBlock;
    }

    ui->plainTextEdit_Paper->moveCursor(QTextCursor::End);
}
