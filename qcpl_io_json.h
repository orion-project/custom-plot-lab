#ifndef QCPL_IO_JSON_H
#define QCPL_IO_JSON_H

#include <QString>
#include <QPen>

class QCPAxis;
class QCPColorScale;
class QCPGraph;
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

QJsonObject writePlot(Plot *plot);
QJsonObject writeLegend(QCPLegend* legend);
QJsonObject writeTitle(QCPTextElement* title);
QJsonObject writeAxis(QCPAxis *axis);
QJsonObject writeColorScale(QCPColorScale *scale);
QJsonObject writeGraph(QCPGraph * graph);
QJsonObject writePen(const QPen& pen);

void readPlot(const QJsonObject& root, Plot *plot, JsonReport* report);
JsonError readLegend(const QJsonObject &obj, QCPLegend* legend);
JsonError readTitle(const QJsonObject &obj, QCPTextElement* title);
JsonError readAxis(const QJsonObject &obj, QCPAxis* axis);
JsonError readColorScale(const QJsonObject &obj, QCPColorScale *scale);
JsonError readGraph(const QJsonObject &obj, QCPGraph * graph);
QPen readPen(const QJsonObject& obj, const QPen& def);

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
    virtual void saveColorScale(QCPColorScale* scale) = 0;
};

/**
    Default implementation of FormatSaver that stores plot format
    in local INI settings as JSON strings.
*/
class FormatStorageIni: public FormatSaver
{
public:
    void save(Plot* plot);
    void load(Plot* plot, JsonReport *report);

    void saveLegend(QCPLegend* legend) override;
    void saveTitle(QCPTextElement* title) override;
    void saveAxis(QCPAxis* axis) override;
    void saveColorScale(QCPColorScale* scale) override;
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

void copyPlotFormat(Plot* plot);
void copyLegendFormat(QCPLegend* legend);
void copyTitleFormat(QCPTextElement* title);
void copyAxisFormat(QCPAxis* axis);
void copyColorScaleFormat(QCPColorScale* scale);
void copyGraphFormat(QCPGraph* graph);

// These commands are for context menus and hence should be invoked on visible elements.
// It's not expected that element gets hidden when its format pasted
// so the functions don't change visibility
QString pastePlotFormat(Plot* plot);
QString pasteLegendFormat(QCPLegend* legend);
QString pasteTitleFormat(QCPTextElement* title);
QString pasteAxisFormat(QCPAxis* axis);
QString pasteColorScaleFormat(QCPColorScale* scale);
QString pasteGraphFormat(QCPGraph* graph);

} // namespace QCPL

#endif // QCPL_IO_JSON_H
