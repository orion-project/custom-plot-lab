#include "qcpl_types.h"

#include <cmath>

namespace QCPL {

//------------------------------------------------------------------------------
//                             AxisLimits
//------------------------------------------------------------------------------

bool AxisLimits::isInvalid() const
{
    return std::isnan(min) or std::isnan(max);
}

QString AxisLimits::str() const
{
    return QString("%1 - %2").arg(min).arg(max);
}

//------------------------------------------------------------------------------
//                             AxisLimits
//------------------------------------------------------------------------------

bool isAxisFactorSet(const AxisFactor& factor)
{
    if (std::holds_alternative<int>(factor))
        return std::get<int>(factor) != 0;
    if (std::holds_alternative<double>(factor))
    {
        auto f = std::get<double>(factor);
        return f != 0.0 && f != 1.0;
    }
    return false;
}

QString axisFactorStr(const AxisFactor& factor)
{
    if (!isAxisFactorSet(factor))
        return QString();
    if (std::holds_alternative<int>(factor))
        return QString("×1e%1").arg(std::get<int>(factor));
    if (std::holds_alternative<double>(factor))
        return QString("×%1").arg(std::get<double>(factor));
    return QString();
}

} // namespace QCPL
