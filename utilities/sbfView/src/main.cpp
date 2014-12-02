#include <QApplication>
#include "sbfviewmainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SbfViewMainWindow w;
    w.show();

    return a.exec();
}
