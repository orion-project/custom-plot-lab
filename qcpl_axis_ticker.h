#ifndef QCPL_AXIS_TICKER_H
#define QCPL_AXIS_TICKER_H

#include "qcustomplot/qcustomplot.h"

namespace QCPL {

class FactorAxisTicker : public QCPAxisTicker
{
public:
    FactorAxisTicker(QSharedPointer<QCPAxisTicker> prevTicker);

    void generate(const QCPRange &range, const QLocale &locale, QChar formatChar, int precision, QVector<double> &ticks, QVector<double> *subTicks, QVector<QString> *tickLabels) override;

    int factor = 0;
    QSharedPointer<QCPAxisTicker> prevTicker;
};

} // namespace QCPL

#endif // QCPL_AXIS_TICKER_H
