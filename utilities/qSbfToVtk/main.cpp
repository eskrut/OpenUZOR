#include <QApplication>
#include "sbftovtkdialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SbfToVtkDialog w;
    w.show();
    
    return a.exec();
}
