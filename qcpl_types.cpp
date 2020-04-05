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

} // namespace QCPL
