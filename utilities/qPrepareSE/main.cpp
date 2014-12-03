#include <QApplication>
#include <QTranslator>
#include "qpreparese.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator *translator = new QTranslator;
    translator->load("ru_ru.qm");
    a.installTranslator(translator);
    QPrepareSE w;
    w.show();

    return a.exec();
}
