#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("MZSoftwareGmbH"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("mzsoft.de"));
    QCoreApplication::setApplicationName(QStringLiteral("C64Foundry"));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "C64Foundry_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            application.installTranslator(&translator);
            break;
        }
    }

    MainWindow mainWindow;
    if (!mainWindow.initialize())
        return 1;

    mainWindow.show();

    return QApplication::exec();
}
