#include "Sandbox.h"

#include "qcpl_format.h"
#include "qcpl_utils.h"

#include "helpers/OriDialogs.h"
#include "tools/OriPetname.h"
#include "tools/OriSettings.h"

PlotWindow::PlotWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("custom-plot-lab sandbox");

    _plot = new QCPL::Plot;
    _plot->formatSaver.reset(new QCPL::FormatStorageIni);
    setCentralWidget(_plot);

    auto m = menuBar()->addMenu("Data");
    m->addAction("Add random graph", this, &PlotWindow::addRandomSample);
    m->addAction("Save plot format...", this, &PlotWindow::savePlotFormat);
    m->addAction("Load plot format...", this, &PlotWindow::loadPlotFormat);
    m->addAction("Load default format", this, &PlotWindow::loadDefaultFormat);

    m = menuBar()->addMenu("Limits");
    m->addAction("Auto", this, [this]{ _plot->autolimits(); });
    m->addAction("Auto X", this, [this]{ _plot->autolimitsX(); });
    m->addAction("Auto Y", this, [this]{ _plot->autolimitsY(); });
    m->addAction("Limits...", this, [this]{ _plot->limitsDlgXY(); });
    m->addAction("Limits X...", this, [this]{ _plot->limitsDlgX(); });
    m->addAction("Limits Y...", this, [this]{ _plot->limitsDlgY(); });

    m = menuBar()->addMenu("Format");
    m->addAction("Plot format...", this, [this]{ QCPL::plotFormatDlg(_plot); });
    m->addAction("Legend format...", this, [this]{ _plot->formatDlgLegend(); });
    m->addAction("X-axis title...", this, [this]{ _plot->titleDlgX(); });
    m->addAction("Y-axis title...", this, [this]{ _plot->titleDlgY(); });
    m->addAction("X-axis format...", this, [this]{ _plot->formatDlgX(); });
    m->addAction("Y-axis format...", this, [this]{ _plot->formatDlgY(); });

    // Example of usage of text variables
    _plot->addTextVar(_plot->xAxis, "{var1}", "Short var 1", []{ return "Galenium overloader"; });
    _plot->addTextVar(_plot->xAxis, "{var2}", "Longer variable name 2", []{ return "mm"; });
    auto getWndW = [this]{ return QString::number(width()); };
    auto getWndH = [this]{ return QString::number(height()); };
    _plot->addTextVar(_plot->xAxis, "{wndW}", "Main window width", getWndW);
    _plot->addTextVar(_plot->xAxis, "{wndH}", "Main window height", getWndH);
    _plot->addTextVar(_plot->yAxis, "{wndW}", "Main window width", getWndW);
    _plot->addTextVar(_plot->yAxis, "{wndH}", "Main window height", getWndH);

    addRandomSample();
    _plot->autolimits();

    resize(800, 600);

    Ori::Settings s;
    recentFormatFile = s.value("recentFormatFile").toString();
}

PlotWindow::~PlotWindow()
{
    Ori::Settings s;
    s.setValue("recentFormatFile", recentFormatFile);
}

void PlotWindow::addRandomSample()
{
    _plot->makeNewGraph(OriPetname::make(), QCPL::makeRandomSample());
}

void PlotWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Example of usage of text variables
    _plot->formatter(_plot->xAxis)->format();
    _plot->formatter(_plot->yAxis)->format();
}

void PlotWindow::savePlotFormat()
{
    auto fileName = QFileDialog::getSaveFileName(
        this, "Save Plot Format", recentFormatFile, "JSON files (*.json)\nAll files (*.*)");
    if (fileName.isEmpty())
        return;
    recentFormatFile = fileName;
    QString res = QCPL::saveFormatToFile(fileName, _plot);
    if (!res.isEmpty())
        Ori::Dlg::error(res);
}

void PlotWindow::loadPlotFormat()
{
    auto fileName = QFileDialog::getOpenFileName(
        this, "Load Plot Format", recentFormatFile, "JSON files (*.json)\nAll files (*.*)");
    if (fileName.isEmpty())
        return;
    recentFormatFile = fileName;
    QString res = QCPL::loadFormatFromFile(fileName, _plot);
    if (!res.isEmpty())
        Ori::Dlg::error(res);
    else
        _plot->replot();
}

void PlotWindow::loadDefaultFormat()
{
    dynamic_cast<QCPL::FormatStorageIni*>(_plot->formatSaver.get())->load(_plot);
    _plot->replot();
}
