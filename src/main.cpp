#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString	locale = QLocale::system().name();
    QTranslator	translator;
    if(translator.load(QString::fromLatin1(":/translates/plotter_").append(locale)))
        a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
