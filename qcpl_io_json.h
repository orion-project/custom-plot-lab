#ifndef QCPL_IO_JSON_H
#define QCPL_IO_JSON_H

#include <QString>

class QCPLegend;

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

namespace QCPL {

void writeLegend(QJsonObject& root, QCPLegend* legend);

} // namespace QCPL

#endif // QCPL_IO_JSON_H
