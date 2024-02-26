#ifndef QCPL_AXIS_TICKER_H
#define QCPL_AXIS_TICKER_H

#include "qcustomplot/qcustomplot.h"

#include "qcpl_types.h"

namespace QCPL {

class FactorAxisTicker : public QCPAxisTicker
{
public:
    FactorAxisTicker(QSharedPointer<QCPAxisTicker> prevTicker);

    void generate(const QCPRange &range, const QLocale &locale, QChar formatChar, int precision,
                    QVector<double> &ticks, QVector<double> *subTicks, QVector<QString> *tickLabels) override;

    AxisFactor factor;
    QSharedPointer<QCPAxisTicker> prevTicker;
};

} // namespace QCPL

#endif // QCPL_AXIS_TICKER_H
