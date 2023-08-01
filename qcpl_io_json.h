#ifndef QCPL_IO_JSON_H
#define QCPL_IO_JSON_H

#include <QString>

class QCPLegend;

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

namespace QCPL {

struct JsonResult
{
    enum { OK, KeyNotFound, BadVersion } code = OK;
    QString message;

    bool ok() { return code == OK; }
};

void writeLegend(QJsonObject& root, QCPLegend* legend);

JsonResult readLegend(const QJsonObject &root, QCPLegend* legend);
JsonResult readLegendObj(const QJsonObject& obj, QCPLegend* legend);

} // namespace QCPL

#endif // QCPL_IO_JSON_H
