#ifndef QCPL_AXIS_FACTOR_H
#define QCPL_AXIS_FACTOR_H

#include "qcustomplot/qcustomplot.h"

#include "qcpl_types.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QGroupBox;
QT_END_NAMESPACE

namespace Ori::Widgets {
class ValueEdit;
}

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

class AxisFactorWidget : public QWidget
{
    Q_OBJECT

public:
    struct Props
    {

    };

    AxisFactorWidget(QCPAxis *axis, const Props& props);

    AxisFactor selectedFactor() const;

private:
    QGroupBox *_groupMetric, *_groupCustom;
    QComboBox *_comboMetric;
    Ori::Widgets::ValueEdit *_editorCustom;

    void updateFocus();
};

} // namespace QCPL

#endif // QCPL_AXIS_FACTOR_H
