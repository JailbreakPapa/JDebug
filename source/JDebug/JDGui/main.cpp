#include "jdgui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JDGui w;
    w.show();
    return a.exec();
}
