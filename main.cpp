#include "gifc_mainwindow.h"

#include "QStyleManager.h"
#include <QApplication>
#include <QFile>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication App(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "GifClipper_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            App.installTranslator(&translator);
            break;
        }
    }

    QStyleManager &styleManager = QStyleManager::getInstance();
    styleManager.setStyle(&App, QStyleManager::QAppStyle::Dark);

    GifC_MainWindow MainWindow;
    MainWindow.show();

    return App.exec();
}
