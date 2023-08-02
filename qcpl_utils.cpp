#include "qcpl_types.h"

#include "qcustomplot/qcustomplot.h"

#include <QRandomGenerator>

namespace QCPL {

GraphData makeRandomSample(int count, double height)
{
    QVector<double> xs(count);
    QVector<double> ys(count);
    auto rnd = QRandomGenerator::global();
    double y = (rnd->generate()%100)*height*0.01;
    for (int i = 0; i < count; i++)
    {
        y = qAbs(y + (rnd->generate()%100)*height*0.01 - height*0.5);
        xs[i] = i;
        ys[i] = y;
    }
    return {xs, ys};
}

Qt::Alignment legendLocation(QCPLegend* legend)
{
    // https://www.qcustomplot.com/index.php/tutorials/basicplotting
    // by default, the legend is in the inset layout of the main axis rect.
    // So this is how we access it to change legend placement:
    return legend->parentPlot()->axisRect()->insetLayout()->insetAlignment(0);
}

void setLegendLocation(QCPLegend* legend, Qt::Alignment align)
{
    legend->parentPlot()->axisRect()->insetLayout()->setInsetAlignment(0, align);
}

QMargins legendMargins(QCPLegend* legend)
{
    return legend->parentPlot()->axisRect()->insetLayout()->margins();
}

void setLegendMargins(QCPLegend* legend, const QMargins& margins)
{
    legend->parentPlot()->axisRect()->insetLayout()->setMargins(margins);
}

} // namespace QCPL
