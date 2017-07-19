#include "qcpl_plot.h"

namespace QCPL {

Plot::Plot()
{
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
void Plot::autolimits(bool autoReplot)
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
    if (autoReplot) replot();
}

} // namespace QCPL

