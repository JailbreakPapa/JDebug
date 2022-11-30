#include "DebuggerGUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DebuggerGUI w;
    w.show();
    return a.exec();
}
