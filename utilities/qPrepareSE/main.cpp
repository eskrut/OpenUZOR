#include <QApplication>
#include "qpreparese.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPrepareSE w;
    w.show();

    return a.exec();
}
