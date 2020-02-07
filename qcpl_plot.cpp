#include "qcpl_plot.h"
#include "qcpl_colors.h"
#include "qcpl_graph.h"

namespace QCPL {

Plot::Plot()
{
    if (!LineGraph::sharedSelectionDecorator())
    {
        // TODO: make selector customizable: line color/width/visibility, points count/color/size/visibility
        auto decorator = new QCPSelectionDecorator;
        decorator->setPen(QPen(QBrush(QColor(0, 255, 255, 120)), 2));
        decorator->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, Qt::black, Qt::black, 6));
        decorator->setUsedScatterProperties(QCPScatterStyle::spAll);
        LineGraph::setSharedSelectionDecorator(decorator);
    }

    legend->setVisible(true);
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables |
                    QCP::iSelectAxes | QCP::iSelectItems | QCP::iSelectLegend | QCP::iSelectOther);
    connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(plotSelectionChanged()));

    // It seems it never called in QCP 2.0.0.b, see QCustomPlot::mousePressEvent + QCustomPlot::mouseReleaseEvent:
    // mMouseEventLayerable can't be QCPAbstractPlottable because of QCPAbstractPlottable always ignores mousePressEvent.
    //connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int,QMouseEvent*)));
}

void Plot::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);

    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeDrag(xAxis->orientation());
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeDrag(yAxis->orientation());
    else
        axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
}

void Plot::wheelEvent(QWheelEvent *event)
{
    QCustomPlot::wheelEvent(event);

    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeZoom(xAxis->orientation());
    else if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        axisRect()->setRangeZoom(yAxis->orientation());
    else
        axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

void Plot::plotSelectionChanged()
{
    // handle axis and tick labels as one selectable object:
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        xAxis->selectedParts().testFlag(QCPAxis::spTickLabels))
        xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    if (yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        yAxis->selectedParts().testFlag(QCPAxis::spTickLabels))
        yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);

    // selection changes event we click on another point of the same graph, so
    // TODO remember selected graph and only emit signal when it was changhed
    for (int i = 0; i < plottableCount(); i++)
    {
        auto g = dynamic_cast<Graph*>(plottable(i));
        if (g && !isService(g) && g->selected())
        {
            emit graphSelected(g);
            return;
        }
    }
}
/*
void Plot::graphClicked(QCPAbstractPlottable *plottable, int, QMouseEvent *)
{
    auto g = dynamic_cast<Graph*>(plottable);
    if (_serviceGraphs.contains(g)) g = nullptr;
    emit graphSelected(g);
}
*/
void Plot::autolimits()
{
    bool onlyEnlarge = false;
    for (int i = 0; i < graphCount(); i++)
    {
        auto g = graph(i);
        if (g && !_serviceGraphs.contains(g))
        {
            g->rescaleAxes(onlyEnlarge);
            onlyEnlarge = true;
        }
    }
    updatePlot();
}

Graph* Plot::makeNewGraph(const QString& title)
{
    auto g = new LineGraph(xAxis, yAxis);

    if (graphAutoColors)
        g->setPen(nextGraphColor());

    g->setName(title);
    g->setSelectable(_selectionType);
    return g;
}

Graph* Plot::makeNewGraph(const QString &title, const QVector<double> &x, const QVector<double> &y)
{
    auto g = makeNewGraph(title);
    g->setData(x, y);

    updatePlot();

    return g;
}

void Plot::updateGraph(Graph* graph, const QVector<double>& x, const QVector<double>& y)
{
    graph->setData(x, y);
    updatePlot();
}

QColor Plot::nextGraphColor()
{
    if (_nextColorIndex == defaultColorSet().size())
        _nextColorIndex = 0;
    return defaultColorSet().at(_nextColorIndex++);
}

void Plot::updatePlot()
{
    if (_replotEnabled) replot();
}

void Plot::setTitleVisible(bool on)
{
    if (_title && on) return;
    if (on)
    {
        _title = new QCPTextElement(this, "", QFont("sans", 14, QFont::Bold));
        _title->setSelectable(true);
        // TODO restore title format

        plotLayout()->insertRow(0);
        plotLayout()->addElement(0, 0, _title);
        connect(_title, &QCPTextElement::doubleClicked, [this](){ emit editTitleRequest(); });
    }
    else
    {
        // TODO backup title format
        plotLayout()->remove(_title);
        plotLayout()->simplify();
        _title = nullptr;
    }
}

} // namespace QCPL

