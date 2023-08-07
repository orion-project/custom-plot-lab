#include "Sandbox.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
    QApplication a(argc, argv);
    a.setStyle("fusion");
    a.setOrganizationName("orion-project.org");
    PlotWindow w;
    w.show();
    return a.exec();
}
