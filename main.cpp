#include "Sandbox.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("fusion");
    a.setOrganizationName("orion-project.org");
    PlotWindow w;
    w.show();
    return a.exec();
}
