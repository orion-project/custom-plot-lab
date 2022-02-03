#include "Sandbox.h"

PlotWindow::PlotWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("custom-plot-lab sandbox");

    _plot = new QCPL::Plot;
    setCentralWidget(_plot);
}

PlotWindow::~PlotWindow()
{
}

