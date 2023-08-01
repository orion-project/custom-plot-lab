#ifndef QCPL_UTILS_H
#define QCPL_UTILS_H

#include "qcpl_types.h"

#include <QMargins>

class QCPLayoutInset;
class QCPLegend;

namespace QCPL {

GraphData makeRandomSample(int count = 100, double height = 25);

Qt::Alignment legendLocation(QCPLegend* legend);
void setLegendLocation(QCPLegend* legend, Qt::Alignment align);
QMargins legendMargins(QCPLegend* legend);
void setLegendMargins(QCPLegend* legend, const QMargins& margins);

} // namespace QCPL

#endif // QCPL_UTILS_H
