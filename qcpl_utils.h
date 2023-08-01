#ifndef QCPL_UTILS_H
#define QCPL_UTILS_H

#include "qcpl_types.h"

class QCPLayoutInset;
class QCPLegend;

namespace QCPL {

GraphData makeRandomSample(int count = 100, double height = 25);

QCPLayoutInset* legendLayout(QCPLegend* legend);

} // namespace QCPL

#endif // QCPL_UTILS_H
