#ifndef QCPL_IO_JSON_H
#define QCPL_IO_JSON_H

#include <QString>

class QCustomPlot;
class QCPLegend;

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

namespace QCPL {

void readPlot(const QJsonObject& root, QCustomPlot *plot);
void readLegend(const QJsonObject &obj, QCPLegend* legend);

QJsonObject writePlot(QCustomPlot *plot);
QJsonObject writeLegend(QCPLegend* legend);

} // namespace QCPL

#endif // QCPL_IO_JSON_H
