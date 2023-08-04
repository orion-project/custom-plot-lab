#ifndef QCPL_IO_JSON_H
#define QCPL_IO_JSON_H

#include <QString>

class QCPAxis;
class QCPLegend;
class QCPTextElement;

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

namespace QCPL {

class Plot;

struct JsonError
{
    enum { OK, NoData, BadVersion } code = OK;
    QString message;

    bool ok() const { return code == OK; }
};

using JsonReport = QVector<JsonError>;

struct JsonOptions
{
    bool allowTextContent = true;
};

QJsonObject writePlot(Plot *plot, const JsonOptions& opts);
QJsonObject writeLegend(QCPLegend* legend);
QJsonObject writeTitle(QCPTextElement* title, const JsonOptions& opts);
QJsonObject writeAxis(QCPAxis *axis, const JsonOptions& opts);

void readPlot(const QJsonObject& root, Plot *plot, const JsonOptions& opts, JsonReport* report);
JsonError readLegend(const QJsonObject &obj, QCPLegend* legend);
JsonError readTitle(const QJsonObject &obj, QCPTextElement* title, const JsonOptions& opts);
JsonError readAxis(const QJsonObject &obj, QCPAxis* axis, const JsonOptions& opts);

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
    virtual void saveTitle(QCPTextElement* title) = 0;
    virtual void saveAxis(QCPAxis* axis) = 0;
};

/**
    Default implementation of FormatSaver that stores plot format
    in local INI settings as JSON strings.
*/
class FormatStorageIni: public FormatSaver
{
public:
    /// Loads default plot format from local INI settings.
    void load(Plot* plot, JsonReport *report);

    void saveLegend(QCPLegend* legend) override;
    void saveTitle(QCPTextElement* title) override;
    void saveAxis(QCPAxis* axis) override;
};

/**
    Loads plot format settings from file and returns empty string when succeeded.
    If error message is returned this means settings have not beed read and plot unchanged.
    Non critical warnings can be optional put in the report object and later shown in app log.
    Suggesting there errors are rare,
    we don't support localization here and return user readable message in common language.
*/
QString loadFormatFromFile(const QString& fileName, Plot *plot, JsonReport* report);

/**
    Saves plot format settings to file and returns empty string when succeeded.
    Suggesting there errors are rare,
    we don't support localization here and return user readable message in common language.
*/
QString saveFormatToFile(const QString& fileName, Plot *plot);

void copyLegendFormat(QCPLegend* legend);
void copyTitleFormat(QCPTextElement* title);
void copyAxisFormat(QCPAxis* axis);
QString pasteLegendFormat(QCPLegend* legend);
QString pasteTitleFormat(QCPTextElement* title);
QString pasteAxisFormat(QCPAxis* axis);

} // namespace QCPL

#endif // QCPL_IO_JSON_H
