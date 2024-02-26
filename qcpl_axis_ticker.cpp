#include "qcpl_axis_ticker.h"

namespace QCPL {

FactorAxisTicker::FactorAxisTicker(QSharedPointer<QCPAxisTicker> prevTicker) : QCPAxisTicker(), prevTicker(prevTicker)
{
    setTickStepStrategy(prevTicker->tickStepStrategy());
    setTickCount(prevTicker->tickCount());
    setTickOrigin(prevTicker->tickOrigin());
}

void FactorAxisTicker::generate(const QCPRange &range, const QLocale &locale, QChar formatChar, int precision,
                                QVector<double> &ticks, QVector<double> *subTicks, QVector<QString> *tickLabels)
{
    prevTicker->setTickStepStrategy(tickStepStrategy());
    prevTicker->setTickCount(tickCount());
    prevTicker->setTickOrigin(tickOrigin());

    QCPAxisTicker::generate(range, locale, formatChar, precision, ticks, subTicks, tickLabels);

    if (dynamic_cast<QCPAxisTickerLog*>(prevTicker.get()))
        return;

    QVector<double> factoredTicks(ticks.size());
    double f = qPow(10.0, factor);
    for (int i = 0; i < ticks.size(); i++)
        factoredTicks[i] = ticks.at(i) / f;

    if (tickLabels)
        *tickLabels = createLabelVector(factoredTicks, locale, formatChar, precision);
}

} // namespace QCPL
