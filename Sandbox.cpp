#include "Sandbox.h"

#include "tools/OriPetname.h"

PlotWindow::PlotWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("custom-plot-lab sandbox");

    _plot = new QCPL::Plot;
    setCentralWidget(_plot);

    auto m = menuBar()->addMenu("Data");
    m->addAction("Add random graph", this, [this]{ _plot->makeNewGraph(OriPetname::make(), QCPL::makeRandomSample()); });

    m = menuBar()->addMenu("Limits");
    m->addAction("Auto", this, [this]{ _plot->autolimits(); });
    m->addAction("Auto X", this, [this]{ _plot->autolimitsX(); });
    m->addAction("Auto Y", this, [this]{ _plot->autolimitsY(); });

    m = menuBar()->addMenu("Format");
    m->addAction("X-axis title...", this, &PlotWindow::editAxisTitleX);
    m->addAction("Y-axis title...", this, &PlotWindow::editAxisTitleY);
}

PlotWindow::~PlotWindow()
{
}

void PlotWindow::editAxisTitleX()
{

}

void PlotWindow::editAxisTitleY()
{

}
