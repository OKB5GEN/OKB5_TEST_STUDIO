#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //w.setWindowFlags(Qt::Dialog);
    //w.setFixedSize(QSize(1130, 630));
    //w.showMaximized();

    w.show();

    return a.exec();
}
