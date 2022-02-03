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

struct TextVariable
{
    QString name;
    QString descr;
};

using TextVarGetter = std::function<QString()>;

} // namespace QCPL

#endif // QCPL_TYPES_H
