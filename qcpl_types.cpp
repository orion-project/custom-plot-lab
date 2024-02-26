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
        return std::get<double>(factor) != 0.0;
    return false;
}

} // namespace QCPL
