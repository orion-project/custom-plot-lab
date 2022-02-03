#include "Sandbox.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PlotWindow w;
    w.show();
    return a.exec();
}
