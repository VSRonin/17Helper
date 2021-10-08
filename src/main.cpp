#include <QApplication>
#include <QTranslator>
#include <mainwindow.h>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTranslator translator;
    if (translator.load(QLocale(), QLatin1String("17Helper"), QLatin1String("_"), QLatin1String(":/i18n")))
        app.installTranslator(&translator);
    MainWindow w;
    w.show();
    return app.exec();
}
