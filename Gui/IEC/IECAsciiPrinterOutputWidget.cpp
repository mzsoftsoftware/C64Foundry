#include "IECAsciiPrinterOutputWidget.h"
#include "ui_IECAsciiPrinterOutputWidget.h"

#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QGraphicsOpacityEffect>


IECAsciiPrinterOutputWidget::IECAsciiPrinterOutputWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IECAsciiPrinterOutputWidget)
{
    ui->setupUi(this);

    QGraphicsOpacityEffect* ptrPaperEffect = new QGraphicsOpacityEffect(ui->plainTextEdit_Paper);
    ptrPaperEffect->setOpacity(1.0);
    ui->plainTextEdit_Paper->setGraphicsEffect(ptrPaperEffect);
    ui->plainTextEdit_Paper->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->plainTextEdit_Paper->setStyleSheet("QPlainTextEdit { background-color: #D8D8D8; color: black; }");
}

IECAsciiPrinterOutputWidget::~IECAsciiPrinterOutputWidget()
{
    delete ui;
}

void IECAsciiPrinterOutputWidget::setPrinterName(const QString& name)
{
    ui->label_PrinterName->setText(name);
}

void IECAsciiPrinterOutputWidget::setPowered(bool powered)
{
    QGraphicsOpacityEffect* ptrEffect = qobject_cast<QGraphicsOpacityEffect*>(ui->plainTextEdit_Paper->graphicsEffect());
    if (ptrEffect == nullptr)
        return;
    ptrEffect->setOpacity(powered ? 1.0 : 0.55);
}

void IECAsciiPrinterOutputWidget::setOutput(const QByteArray& output)
{
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
