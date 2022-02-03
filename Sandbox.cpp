#include "Sandbox.h"

#include "tools/OriPetname.h"

#include <QRandomGenerator>

PlotWindow::PlotWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("custom-plot-lab sandbox");

    _plot = new QCPL::Plot;
    setCentralWidget(_plot);

    auto menuData = menuBar()->addMenu("Data");
    menuData->addAction("Add random graph", this, &PlotWindow::addRandomGraph);

    auto menuLimits = menuBar()->addMenu("Limits");
    menuLimits->addAction("Auto", this, [this]{ _plot->autolimits(); });
    menuLimits->addAction("Auto X", this, [this]{ _plot->autolimitsX(); });
    menuLimits->addAction("Auto Y", this, [this]{ _plot->autolimitsY(); });
}

PlotWindow::~PlotWindow()
{
}

void PlotWindow::addRandomGraph()
{
    const double H = 25;
    const int count = 100;
    QVector<double> xs(count);
    QVector<double> ys(count);
    auto rnd = QRandomGenerator::global();
    double y = (rnd->generate()%100)*H*0.01;
    for (int i = 0; i < count; i++)
    {
        y = qAbs(y + (rnd->generate()%100)*H*0.01 - H*0.5);
        xs[i] = i;
        ys[i] = y;
    }
    _plot->makeNewGraph(OriPetname::make(), {xs, ys});
}
