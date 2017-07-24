#ifndef QCPL_TYPES_H
#define QCPL_TYPES_H

#include <QVector>

namespace QCPL {

typedef QVector<double> ValueArray;

struct GraphData
{
    ValueArray x, y;
};

} // namespace QCPL

#endif // QCPL_TYPES_H
