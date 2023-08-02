#ifndef QCPL_IO_JSON_H
#define QCPL_IO_JSON_H

#include <QString>

class QCustomPlot;
class QCPLegend;

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

namespace QCPL {

struct JsonError
{
    enum { OK, NoData, BadVersion } code = OK;
    QString message;

    bool ok() const { return code == OK; }
};

using JsonReport = QVector<JsonError>;

QJsonObject writePlot(QCustomPlot *plot);
QJsonObject writeLegend(QCPLegend* legend);

void readPlot(const QJsonObject& root, QCustomPlot *plot, JsonReport* report);
JsonError readLegend(const QJsonObject &obj, QCPLegend* legend);

/**
    Allows a plot to store default view format of its elements.
    The view format is fonts and colors of axes, of legend, etc.
    Make an object of derived class and assign to the Plot::formatSaver field.
*/
class FormatSaver
{
public:
    virtual ~FormatSaver() {}
    virtual void saveLegend(QCPLegend* legend) = 0;
};

/**
    Default implementation of FormatSaver that stores plot format
    in local INI settings as JSON strings.
*/
class FormatStorageIni: public FormatSaver
{
public:
    void load(QCustomPlot* plot, JsonReport *report);
    void saveLegend(QCPLegend* legend) override;
};

/// Loads plot format settings from file and returns empty string when succeeded.
/// If error message is returned this means settings have not beed read and plot unchanged.
/// Non critical warnings can be optional put in the report object and later shown in app log.
/// Suggesting there errors are rare,
/// we don't support localization here and return user readable message in common language.
QString loadFormatFromFile(const QString& fileName, QCustomPlot* plot, JsonReport* report);

/// Saves plot format settings to file and returns empty string when succeeded.
/// Suggesting there errors are rare,
/// we don't support localization here and return user readable message in common language.
QString saveFormatToFile(const QString& fileName, QCustomPlot* plot);

/// Copies legend format settings into Clipboard as JSON text.
void copyLegendFormat(QCPLegend* legend);

/// Tries to parse a text from Clipboard into JSON object and load legend format settings from there.
QString pasteLegendFormat(QCPLegend* legend);

} // namespace QCPL

#endif // QCPL_IO_JSON_H
