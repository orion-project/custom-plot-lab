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
//                             ValueFormatter
//------------------------------------------------------------------------------

ValueFormatter::~ValueFormatter() {}

QString DefaultValueFormatter::format(double v) const
{
    return QString::number(v, 'g', 10);
}

const ValueFormatter* getDefaultValueFormatter()
{
    static const DefaultValueFormatter f;
    return &f;
}

} // namespace QCPL
