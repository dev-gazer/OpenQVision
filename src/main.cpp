#include "mainwindow.h"
#include <QApplication>
#include <curl_telegram.h>
#include <QTextStream>
#include <QFile>
#include <QDataStream>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile f(":qdarkstyle/dark/style.qss");

    if (!f.exists())   {
        printf("Unable to set stylesheet, file not found\n");
    }
    else   {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
    MainWindow w;
    w.show();
    return a.exec();
}
