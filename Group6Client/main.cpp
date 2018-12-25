#include "clientdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientDialog w;
    w.show();

    return a.exec();
}
