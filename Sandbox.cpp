#include "Sandbox.h"

#include "qcpl_format.h"
#include "qcpl_io_json.h"
#include "qcpl_utils.h"

#include "helpers/OriDialogs.h"
#include "tools/OriPetname.h"
#include "tools/OriSettings.h"

PlotWindow::PlotWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("custom-plot-lab sandbox");

    _plot = new QCPL::Plot;
    _plot->formatSaver.reset(new QCPL::FormatStorageIni);
    _plot->legend->setSelectableParts(QCPLegend::spLegendBox);
    setCentralWidget(_plot);

    auto m = menuBar()->addMenu("Data");
    m->addAction("Add random graph", this, &PlotWindow::addRandomSample);
    m->addAction("Save plot format...", this, &PlotWindow::savePlotFormat);
    m->addAction("Load plot format...", this, &PlotWindow::loadPlotFormat);
    m->addAction("Load default plot format", this, &PlotWindow::loadDefaultFormat);

    m = menuBar()->addMenu("Limits");
    m->addAction("Auto", this, [this]{ _plot->autolimits(); });
    m->addAction("Auto X", this, [this]{ _plot->autolimitsX(); });
    m->addAction("Auto Y", this, [this]{ _plot->autolimitsY(); });
    m->addAction("Limits...", this, [this]{ _plot->limitsDlgXY(); });
    m->addAction("Limits X...", this, [this]{ _plot->limitsDlgX(); });
    m->addAction("Limits Y...", this, [this]{ _plot->limitsDlgY(); });

    m = menuBar()->addMenu("Format");
    m->addAction("Plot format...", this, [this]{ QCPL::plotFormatDlg(_plot); });
    m->addAction("Legend format...", this, [this]{ _plot->legendFormatDlg(); });
    m->addSeparator();
    m->addAction("Title format...", this, [this]{ _plot->titleFormatDlg(); });
    m->addAction("Title text...", this, [this]{ _plot->titleTextDlg(); });
    m->addSeparator();
    m->addAction("X-axis format...", this, [this]{ _plot->axisFormatDlgX(); });
    m->addAction("X-axis text...", this, [this]{ _plot->axisTextDlgX(); });
    m->addSeparator();
    m->addAction("Y-axis format...", this, [this]{ _plot->axisFormatDlgY(); });
    m->addAction("Y-axis text...", this, [this]{ _plot->axisTextDlgY(); });

    // Example of usage of text variables
    // Variable getters provide values for variables.
    // They are called when one calls `format()` on partucular TextFormatter object
    auto getWndW = [this]{ return QString::number(width()); };
    auto getWndH = [this]{ return QString::number(height()); };
    _plot->addTextVarT("{text_var}", "Text var 3", []{ return "Hallo World"; });
    _plot->addTextVarT("{unit_1}", "Unit of measurement 1", []{ return "mm"; });
    _plot->addTextVarT("{unit_2}", "Unit of measurement 2", []{ return "kg"; });
    _plot->addTextVarT("{wndW}", "Main window width", getWndW);
    _plot->addTextVarT("{wndH}", "Main window height", getWndH);

    _plot->addTextVarX("{some_text}", "Text var 1", []{ return "Galenium overloader"; });
    _plot->addTextVarX("{unit_1}", "Unit of measurement 1", []{ return "mm"; });
    _plot->addTextVarX("{unit_2}", "Unit of measurement 2", []{ return "kg"; });
    _plot->addTextVarX("{wndW}", "Main window width", getWndW);
    _plot->addTextVarX("{wndH}", "Main window height", getWndH);

    _plot->addTextVarY("{text_var}", "Text var 2", []{ return "Lorem ipsum"; });
    _plot->addTextVarY("{unit_1}", "Unit of measurement 1", []{ return "mm"; });
    _plot->addTextVarY("{unit_2}", "Unit of measurement 2", []{ return "kg"; });
    _plot->addTextVarY("{wndW}", "Main window width", getWndW);
    _plot->addTextVarY("{wndH}", "Main window height", getWndH);

    _plot->setDefaultTextT("Plot title");
    _plot->setDefaultTextX("Axis X title");
    _plot->setDefaultTextY("Axis Y title");

    _plot->menuLegend = new QMenu(this);
    _plot->menuLegend->addAction("Format...", this, [this]{ _plot->legendFormatDlg(); });
    _plot->menuLegend->addAction("Copy format", this, [this](){ QCPL::copyLegendFormat(_plot->legend); });
    _plot->menuLegend->addAction("Paste format", this, &PlotWindow::pasteLegendFormat);
    _plot->menuLegend->addAction("Hide", this, [this](){ _plot->legend->setVisible(false); _plot->replot(); });
    _plot->menuTitle = new QMenu(this);
    _plot->menuTitle->addAction("Format...", this, [this]{ _plot->titleFormatDlg(); });
    _plot->menuTitle->addAction("Text...", this, [this]{ _plot->titleTextDlg(); });
    _plot->menuTitle->addAction("Copy format", this, [this](){ QCPL::copyTitleFormat(_plot->title()); });
    _plot->menuTitle->addAction("Paste format", this, &PlotWindow::pasteTitleFormat);
    _plot->menuTitle->addAction("Hide", this, [this](){ _plot->title()->setVisible(false); _plot->updateTitleVisibility(); _plot->replot(); });
    _plot->menuAxisX = new QMenu(this);
    _plot->menuAxisX->addAction("Format...", this, [this]{ _plot->axisFormatDlgX(); });
    _plot->menuAxisX->addAction("Text...", this, [this]{ _plot->axisTextDlgX(); });
    _plot->menuAxisX->addAction("Copy format", this, [this](){ QCPL::copyAxisFormat(_plot->xAxis); });
    _plot->menuAxisX->addAction("Paste format", this, [this](){ pasteAxisFormat(_plot->xAxis); });
    _plot->menuAxisY = new QMenu(this);
    _plot->menuAxisY->addAction("Format...", this, [this]{ _plot->axisFormatDlgY(); });
    _plot->menuAxisY->addAction("Text...", this, [this]{ _plot->axisTextDlgY(); });
    _plot->menuAxisY->addAction("Copy format", this, [this](){ QCPL::copyAxisFormat(_plot->yAxis); });
    _plot->menuAxisY->addAction("Paste format", this, [this](){ pasteAxisFormat(_plot->yAxis); });

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
    // Update texts if they use variable referring some
    // dynamically changed objects (the window size in this example)
    _plot->updateTexts();
}

void PlotWindow::savePlotFormat()
{
    auto fileName = QFileDialog::getSaveFileName(
        this, "Save Plot Format", recentFormatFile, "JSON files (*.json)\nAll files (*.*)");
    if (fileName.isEmpty())
        return;
    recentFormatFile = fileName;
    QString err = QCPL::saveFormatToFile(fileName, _plot);
    if (!err.isEmpty())
        Ori::Dlg::error(err);
}

void PlotWindow::loadPlotFormat()
{
    auto fileName = QFileDialog::getOpenFileName(
        this, "Load Plot Format", recentFormatFile, "JSON files (*.json)\nAll files (*.*)");
    if (fileName.isEmpty())
        return;
    recentFormatFile = fileName;

    QCPL::JsonReport report;
    auto err = QCPL::loadFormatFromFile(fileName, _plot, &report);
    if (!err.isEmpty())
    {
        // The plot definitely has not been changed
        Ori::Dlg::error(err);
        return;
    }

    // The plot probably has not been changed, this can be clarified by examining the report
    _plot->replot();

    // In real app these messages could be shown in app log window, for example
    foreach (auto err, report)
        qDebug() << err.message;
}

void PlotWindow::loadDefaultFormat()
{
    QCPL::JsonReport report;
    dynamic_cast<QCPL::FormatStorageIni*>(_plot->formatSaver.get())->load(_plot, &report);

    // The plot probably has not been changed, this can be clarified by examining the report
    _plot->replot();

    // In real app these messages could be shown in app log window, for example
    foreach (auto err, report)
        qDebug() << err.message;
}

void PlotWindow::pasteLegendFormat()
{
    auto err = QCPL::pasteLegendFormat(_plot->legend);
    if (err.isEmpty())
    {
        qDebug() << "Legend format pasted";
        _plot->replot();
    }
    else Ori::Dlg::error(err);
}

void PlotWindow::pasteTitleFormat()
{
    auto err = QCPL::pasteTitleFormat(_plot->title());
    if (err.isEmpty())
    {
        qDebug() << "Title format pasted";
        // This is a context menu command and it's done on visible element.
        // It's not expected that element gets hidden when its format pasted.
        // So the `QCPL::pasteTitleFormat()` doesn't change visibility
        // and there is not need to call `Plot::updateTitleVisibility()`
        _plot->replot();
    }
    else Ori::Dlg::error(err);
}

void PlotWindow::pasteAxisFormat(QCPAxis *axis)
{
    auto err = QCPL::pasteAxisFormat(axis);
    if (err.isEmpty())
    {
        qDebug() << "Axis format pasted";
        _plot->replot();
    }
    else Ori::Dlg::error(err);
}
