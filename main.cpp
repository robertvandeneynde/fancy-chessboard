#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    // MyGLDrawer w;
    // Window w;
    w.show();

    return a.exec();
}
