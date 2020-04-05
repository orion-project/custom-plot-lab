#ifndef QCPL_TYPES_H
#define QCPL_TYPES_H

#include <QVector>

namespace QCPL {

typedef QVector<double> ValueArray;

struct GraphData
{
    ValueArray x, y;
};

struct AxisLimits
{
    double min;
    double max;

    AxisLimits() { min = 0; max = 0; }
    AxisLimits(double min, double max) { this->min = min; this->max = max; }

    bool isInvalid() const;

    QString str() const;
};

class ValueFormatter
{
public:
    virtual ~ValueFormatter();
    virtual bool isDecimalComma() const = 0;
    virtual QString format(double v) const = 0;
};

class DefaultValueFormatter : public ValueFormatter
{
public:
    bool isDecimalComma() const override { return false; }
    QString format(double v) const override;
};

const ValueFormatter* getDefaultValueFormatter();

} // namespace QCPL

#endif // QCPL_TYPES_H
