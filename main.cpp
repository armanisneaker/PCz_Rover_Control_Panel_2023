#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("fusion");
    qputenv("QT_SCALE_FACTOR", QByteArray("1"));
    MainWindow w;
    w.show();
    return a.exec();
}
