#ifndef QCPL_TYPES_H
#define QCPL_TYPES_H

#include <QVector>
#include <QFont>

#include <variant>

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

struct PlotFormatSettings
{
    QString fontName;
    int axisTitleFontSize = 0;
    int axisTicksFontSize = 0;
};

using AxisFactor = std::variant<int, double>;
bool isAxisFactorSet(const AxisFactor& factor);
QString axisFactorStr(const AxisFactor& factor);

} // namespace QCPL

#endif // QCPL_TYPES_H
