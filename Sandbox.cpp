#include "Sandbox.h"

#include "tools/OriPetname.h"

PlotWindow::PlotWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("custom-plot-lab sandbox");

    _plot = new QCPL::Plot;
    setCentralWidget(_plot);

    auto m = menuBar()->addMenu("Data");
    m->addAction("Add random graph", this, &PlotWindow::addRandomSample);

    m = menuBar()->addMenu("Limits");
    m->addAction("Auto", this, [this]{ _plot->autolimits(); });
    m->addAction("Auto X", this, [this]{ _plot->autolimitsX(); });
    m->addAction("Auto Y", this, [this]{ _plot->autolimitsY(); });
    m->addAction("Limits...", this, [this]{ _plot->limitsDlgXY(); });
    m->addAction("Limits X...", this, [this]{ _plot->limitsDlgX(); });
    m->addAction("Limits Y...", this, [this]{ _plot->limitsDlgY(); });

    m = menuBar()->addMenu("Format");
    m->addAction("X-axis title...", this, [this]{ _plot->titleDlgX(); });
    m->addAction("Y-axis title...", this, [this]{ _plot->titleDlgY(); });

    _plot->addTextVar(_plot->xAxis, "{var1}", "Short var 1", []{ return "Galenium overloader"; });
    _plot->addTextVar(_plot->xAxis, "{var2}", "Longer variable name 2", []{ return "mm"; });

    addRandomSample();
    _plot->autolimits();

    resize(800, 600);
}

PlotWindow::~PlotWindow()
{
}

void PlotWindow::addRandomSample()
{
    _plot->makeNewGraph(OriPetname::make(), QCPL::makeRandomSample());
}
