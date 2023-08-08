#include "qcpl_io_json.h"

#include "qcpl_plot.h"
#include "qcpl_utils.h"

#include "core/OriResult.h"
#include "tools/OriSettings.h"

#define MIME_TYPE "application/x-orion-project-org;value=plot-format"
#define CURRENT_LEGEND_VERSION 1
#define CURRENT_TITLE_VERSION 1
#define CURRENT_AXIS_VERSION 1
#define SECTION_INI "DefaultPlotFormat"
#define KEY_TITLE "title"
#define KEY_LEGEND "legend"
#define KEY_AXIS "axis"
#define KEY_AXIS_X "axis_x"
#define KEY_AXIS_Y "axis_y"

namespace QCPL {

namespace {

QJsonObject writeFont(const QFont& font)
{
    return QJsonObject({
        { "family", font.family() },
        { "size", font.pointSize() },
        { "bold", font.bold() },
        { "italic", font.italic() },
        { "underline", font.underline() },
        { "strikeout", font.strikeOut() },
    });
}

QFont readFont(const QJsonObject& obj, const QFont& def)
{
    QFont f(def);
    f.setFamily(obj["family"].toString(def.family()));
    f.setPointSize(obj["size"].toInt(def.pointSize()));
    f.setBold(obj["bold"].toBool(def.bold()));
    f.setItalic(obj["italic"].toBool(def.italic()));
    f.setUnderline(obj["underline"].toBool(def.underline()));
    f.setStrikeOut(obj["strikeout"].toBool(def.strikeOut()));
    return f;
}

QJsonObject writeSize(const QSize& size)
{
    return QJsonObject({
        { "width", size.width() },
        { "height", size.height() },
    });
}

QSize readSize(const QJsonObject& obj, const QSize& def)
{
    return QSize(
        obj["width"].toInt(def.width()),
        obj["height"].toInt(def.height())
    );
}

QJsonObject writeMargins(const QMargins& margins)
{
    return QJsonObject({
        { "left", margins.left() },
        { "top", margins.top() },
        { "right", margins.right() },
        { "bottom", margins.bottom() },
    });
}

QMargins readMargins(const QJsonObject& obj, const QMargins& def)
{
    return QMargins(
        obj["left"].toInt(def.left()),
        obj["top"].toInt(def.top()),
        obj["right"].toInt(def.right()),
        obj["bottom"].toInt(def.bottom())
    );
}

QJsonObject writePen(const QPen& pen)
{
    return QJsonObject({
        { "color", pen.color().name() },
        { "style", int(pen.style()) },
        { "width", pen.width() },
    });
}

QPen readPen(const QJsonObject& obj, const QPen& def)
{
    QPen p(def);
    QColor c(obj["color"].toString());
    if (c.isValid())
        p.setColor(c);
    p.setStyle(Qt::PenStyle(obj["style"].toInt(def.style())));
    p.setWidth(obj["width"].toInt(def.width()));
    return p;
}

QJsonValue colorToJson(const QColor& color)
{
    return color.name();
}

QColor jsonToColor(const QJsonValue& val, const QColor& def)
{
    QColor color(val.toString());
    return color.isValid() ? color : def;
}

} // namespace

//------------------------------------------------------------------------------
//                             Qrite to JSON

QJsonObject writePlot(Plot* plot)
{
    return QJsonObject({
        { KEY_LEGEND, writeLegend(plot->legend) },
        { KEY_TITLE, writeTitle(plot->title()) },
        { KEY_AXIS_X, writeAxis(plot->xAxis) },
        { KEY_AXIS_Y, writeAxis(plot->yAxis) },
    });
}

QJsonObject writeLegend(QCPLegend* legend)
{
    return QJsonObject({
        { "version", CURRENT_LEGEND_VERSION },
        { "visible", legend->visible() },
        { "back_color", legend->brush().color().name() },
        { "text_color", legend->textColor().name() },
        { "font", writeFont(legend->font()) },
        { "icon_size", writeSize(legend->iconSize()) },
        { "icon_margin", legend->iconTextPadding() },
        { "border", writePen(legend->borderPen()) },
        { "paddings", writeMargins(legend->margins()) },
        { "margins", writeMargins(legendMargins(legend)) },
        { "location", int(legendLocation(legend)) }
    });
}

QJsonObject writeTitle(QCPTextElement* title)
{
    auto obj = QJsonObject({
        { "version", CURRENT_TITLE_VERSION },
        { "visible", title->visible() },
        { "font", writeFont(title->font()) },
        { "text_color", colorToJson(title->textColor()) },
        { "text_flags", title->textFlags() },
        { "margins", writeMargins(title->margins()) },
    });
    return obj;
}

QJsonObject writeAxis(QCPAxis *axis)
{
    auto grid = axis->grid();
    auto obj = QJsonObject({
        { "version", CURRENT_AXIS_VERSION },
        { "visible", axis->visible() },
        { "title_font", writeFont(axis->labelFont()) },
        { "title_color", colorToJson(axis->labelColor()) },
        { "title_margin_in", axis->labelPadding() },
        { "title_margin_out", axis->padding() },
        { "offset", axis->offset() },
        { "scale_log", axis->scaleType() == QCPAxis::stLogarithmic },
        { "reversed", axis->rangeReversed() },
        { "labels_visible", axis->tickLabels() },
        { "labels_inside", axis->tickLabelSide() == QCPAxis::lsInside },
        { "labels_rotation", axis->tickLabelRotation() },
        { "labels_margin", axis->tickLabelPadding() },
        { "labels_color", colorToJson(axis->tickLabelColor()) },
        { "labels_font", writeFont(axis->tickLabelFont()) },
        { "number_format", axis->numberFormat() },
        { "number_precision", axis->numberPrecision() },
        { "pen", writePen(axis->basePen()) },
        { "tick_visible", axis->ticks() },
        { "tick_pen", writePen(axis->tickPen()) },
        { "tick_len_in", axis->tickLengthIn() },
        { "tick_len_out", axis->tickLengthOut() },
        { "subtick_visible", axis->subTicks() },
        { "subtick_pen", writePen(axis->subTickPen()) },
        { "subtick_len_in", axis->subTickLengthIn() },
        { "subtick_len_out", axis->subTickLengthOut() },
        { "grid_visible", grid->visible() },
        { "grid_pen", writePen(grid->pen()) },
        { "zero_pen", writePen(grid->zeroLinePen()) },
        { "subgrid_visible", grid->subGridVisible() },
        { "subgrid_pen", writePen(grid->subGridPen()) },
    });
    return obj;
}

//------------------------------------------------------------------------------
//                             Read from JSON

void readPlot(const QJsonObject& root, Plot *plot, JsonReport *report)
{
    if (auto err = readLegend(root[KEY_LEGEND].toObject(), plot->legend); !err.ok() and report)
        report->append(err);
    if (auto err = readTitle(root[KEY_TITLE].toObject(), plot->title()); !err.ok() and report)
        report->append(err);
    if (auto err = readAxis(root[KEY_AXIS_X].toObject(), plot->xAxis); !err.ok() and report)
        report->append(err);
    if (auto err = readAxis(root[KEY_AXIS_Y].toObject(), plot->yAxis); !err.ok() and report)
        report->append(err);
    plot->updateTitleVisibility();
}

JsonError readLegend(const QJsonObject& obj, QCPLegend* legend)
{
    if (obj.isEmpty())
        return { JsonError::NoData, "Legend object is empty" };
    auto ver = obj["version"].toInt();
    if (ver != CURRENT_LEGEND_VERSION)
        return {
            JsonError::BadVersion,
            QString("Unsupported legend version %1, expected %2").arg(ver, CURRENT_LEGEND_VERSION) };
    legend->setVisible(obj["visible"].toBool(legend->visible()));
    legend->setBrush(jsonToColor(obj["back_color"], legend->brush().color()));
    legend->setTextColor(jsonToColor(obj["text_color"], legend->textColor()));
    legend->setFont(readFont(obj["font"].toObject(), legend->font()));
    legend->setSelectedFont(legend->font());
    legend->setIconSize(readSize(obj["icon_size"].toObject(), legend->iconSize()));
    legend->setIconTextPadding(obj["icon_margin"].toInt(legend->iconTextPadding()));
    legend->setBorderPen(readPen(obj["border"].toObject(), legend->borderPen()));
    legend->setMargins(readMargins(obj["paddings"].toObject(), legend->margins()));
    setLegendMargins(legend, readMargins(obj["margins"].toObject(), legendMargins(legend)));
    setLegendLocation(legend, Qt::Alignment(obj["location"].toInt(legendLocation(legend))));
    return {};
}

JsonError readTitle(const QJsonObject &obj, QCPTextElement* title)
{
    if (obj.isEmpty())
        return { JsonError::NoData, "Title object is empty" };
    auto ver = obj["version"].toInt();
    if (ver != CURRENT_TITLE_VERSION)
        return {
                JsonError::BadVersion,
                QString("Unsupported title version %1, expected %2").arg(ver, CURRENT_TITLE_VERSION) };
    title->setVisible(obj["visible"].toBool(title->visible()));
    title->setFont(readFont(obj["font"].toObject(), title->font()));
    title->setSelectedFont(title->font());
    title->setTextColor(jsonToColor(obj["text_color"], title->textColor()));
    title->setTextFlags(obj["text_flags"].toInt(title->textFlags()));
    title->setMargins(readMargins(obj["margins"].toObject(), title->margins()));
    return {};
}

JsonError readAxis(const QJsonObject &obj, QCPAxis* axis)
{
    if (obj.isEmpty())
        return { JsonError::NoData, "Axis object is empty" };
    auto ver = obj["version"].toInt();
    if (ver != CURRENT_AXIS_VERSION)
        return {
                JsonError::BadVersion,
                QString("Unsupported axis version %1, expected %2").arg(ver, CURRENT_AXIS_VERSION) };
    axis->setVisible(obj["visible"].toBool(axis->visible()));
    axis->setLabelFont(readFont(obj["title_font"].toObject(), axis->labelFont()));
    axis->setSelectedLabelFont(axis->labelFont());
    axis->setLabelColor(jsonToColor(obj["title_color"], axis->labelColor()));
    axis->setLabelPadding(obj["title_margin_in"].toInt(axis->labelPadding()));
    axis->setPadding(obj["title_margin_out"].toInt(axis->padding()));
    axis->setOffset(obj["offset"].toInt(axis->offset()));
    axis->setScaleType(obj["scale_log"].toBool(axis->scaleType() == QCPAxis::stLogarithmic) ? QCPAxis::stLogarithmic : QCPAxis::stLinear);
    axis->setRangeReversed(obj["reversed"].toBool(axis->rangeReversed()));
    axis->setTickLabels(obj["labels_visible"].toBool(axis->tickLabels()));
    axis->setTickLabelSide(obj["labels_inside"].toBool(axis->tickLabelSide() == QCPAxis::lsInside) ? QCPAxis::lsInside : QCPAxis::lsOutside);
    axis->setTickLabelRotation(obj["labels_rotation"].toDouble(axis->tickLabelRotation()));
    axis->setTickLabelPadding(obj["labels_margin"].toInt(axis->tickLabelPadding()));
    axis->setTickLabelColor(jsonToColor(obj["labels_color"], axis->tickLabelColor()));
    axis->setTickLabelFont(readFont(obj["labels_font"].toObject(), axis->tickLabelFont()));
    axis->setSelectedTickLabelFont(axis->tickLabelFont());
    axis->setNumberFormat(obj["number_format"].toString(axis->numberFormat()));
    axis->setNumberPrecision(obj["number_precision"].toInt(axis->numberPrecision()));
    axis->setBasePen(readPen(obj["pen"].toObject(), axis->basePen()));
    axis->setTicks(obj["tick_visible"].toBool(axis->ticks()));
    axis->setTickPen(readPen(obj["tick_pen"].toObject(), axis->tickPen()));
    axis->setTickLengthIn(obj["tick_len_in"].toInt(axis->tickLengthIn()));
    axis->setTickLengthOut(obj["tick_len_out"].toInt(axis->tickLengthOut()));
    axis->setSubTicks(obj["subtick_visible"].toInt(axis->subTicks()));
    axis->setSubTickPen(readPen(obj["subtick_pen"].toObject(), axis->subTickPen()));
    axis->setSubTickLengthIn(obj["subtick_len_in"].toInt(axis->subTickLengthIn()));
    axis->setSubTickLengthOut(obj["subtick_len_out"].toInt(axis->subTickLengthOut()));
    auto grid = axis->grid();
    grid->setVisible(obj["grid_visible"].toBool(grid->visible()));
    grid->setPen(readPen(obj["grid_pen"].toObject(), grid->pen()));
    grid->setZeroLinePen(readPen(obj["zero_pen"].toObject(), grid->zeroLinePen()));
    grid->setSubGridVisible(obj["subgrid_visible"].toBool(grid->subGridVisible()));
    grid->setSubGridPen(readPen(obj["subgrid_pen"].toObject(), grid->subGridPen()));
    return {};
}

//------------------------------------------------------------------------------
//                          QCPL::FormatStorageIni
//------------------------------------------------------------------------------

static QJsonObject varToJson(const QVariant& data)
{
    QString str = data.toString();
    if (str.isEmpty()) return QJsonObject();
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    return doc.isNull() ? QJsonObject() : doc.object();
}

static QVariant jsonToVar(const QJsonObject& obj)
{
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

static void savePlotFormatIni(const QString& key, const QJsonObject& obj)
{
    Ori::Settings s;
    s.beginGroup(SECTION_INI);
    s.setValue(key, jsonToVar(obj));
}

void FormatStorageIni::save(Plot* plot)
{
    Ori::Settings s;
    s.beginGroup(SECTION_INI);
    s.setValue(KEY_TITLE, jsonToVar(writeTitle(plot->title())));
    s.setValue(KEY_LEGEND, jsonToVar(writeLegend(plot->legend)));
    s.setValue(KEY_AXIS_X, jsonToVar(writeAxis(plot->xAxis)));
    s.setValue(KEY_AXIS_Y, jsonToVar(writeAxis(plot->yAxis)));
}

void FormatStorageIni::load(Plot *plot, JsonReport* report)
{
    Ori::Settings s;
    s.beginGroup(SECTION_INI);
    // Non existent settings keys can be safely read too, they result in empty json objects
    // and read functons should skip empty objects without substituting default values for every prop.
    if (auto err = readLegend(varToJson(s.value(KEY_LEGEND)), plot->legend); !err.ok() and report)
        report->append(err);
    if (auto err = readTitle(varToJson(s.value(KEY_TITLE)), plot->title()); !err.ok() and report)
            report->append(err);
    if (auto err = readAxis(varToJson(s.value(KEY_AXIS_X)), plot->xAxis); !err.ok() and report)
        report->append(err);
    if (auto err = readAxis(varToJson(s.value(KEY_AXIS_Y)), plot->yAxis); !err.ok() and report)
        report->append(err);
    plot->updateTitleVisibility();
}

void FormatStorageIni::saveLegend(QCPLegend* legend)
{
    savePlotFormatIni(KEY_LEGEND, writeLegend(legend));
}

void FormatStorageIni::saveTitle(QCPTextElement* title)
{
    savePlotFormatIni(KEY_TITLE, writeTitle(title));
}

void FormatStorageIni::saveAxis(QCPAxis* axis)
{
    auto plot = axis->parentPlot();
    QString key = (axis == plot->xAxis) ? KEY_AXIS_X :
                      ((axis == plot->yAxis) ? KEY_AXIS_Y : KEY_AXIS);
    savePlotFormatIni(key, writeAxis(axis));
}

//------------------------------------------------------------------------------
//                              Load / Save
//------------------------------------------------------------------------------

QString loadFormatFromFile(const QString& fileName, Plot* plot, JsonReport *report)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "Unable to open file for reading: " + file.errorString();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (doc.isNull())
        return "Unable to parse json file: " + error.errorString();

    readPlot(doc.object(), plot, report);
    return {};
}

QString saveFormatToFile(const QString& fileName, Plot* plot)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return "Unable to open file for writing: " + file.errorString();
    QTextStream(&file) << QJsonDocument(writePlot(plot)).toJson();
    return QString();
}

//------------------------------------------------------------------------------
//                          Copy / Paste
//------------------------------------------------------------------------------

static void setClipboardData(const QJsonObject& value, const QString& dataType)
{
    QJsonObject root({{ dataType, value }});
    auto mimeData = new QMimeData;
    mimeData->setData(MIME_TYPE, QJsonDocument(root).toJson());
    qApp->clipboard()->setMimeData(mimeData);
}

using JsonResult = Ori::Result<QJsonObject>;

static JsonResult getClipboradData(const QString& dataType)
{
    auto mimeData = qApp->clipboard()->mimeData();
    if (!mimeData)
        return JsonResult::fail("Clipboard is empty");

    auto data = mimeData->data(MIME_TYPE);
    if (data.isNull())
        return JsonResult::fail("Clipboard doesn't contain data in supported format");

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (doc.isNull())
        return JsonResult::fail("Clipboard text is not a valid JSON text: " + error.errorString());

    QJsonObject root = doc.object();
    if (!root.contains(dataType))
        return JsonResult::fail("There is no data of appropriate type in Clipboard");

    return JsonResult::ok(root[dataType].toObject());
}

void copyPlotFormat(Plot* plot)
{
    setClipboardData(writePlot(plot), "plot");
}

void copyLegendFormat(QCPLegend* legend)
{
    setClipboardData(writeLegend(legend), KEY_LEGEND);
}

void copyTitleFormat(QCPTextElement* title)
{
    setClipboardData(writeTitle(title), KEY_TITLE);
}

void copyAxisFormat(QCPAxis* axis)
{
    setClipboardData(writeAxis(axis), KEY_AXIS);
}

QString pastePlotFormat(Plot* plot)
{
    auto res = getClipboradData("plot");
    if (!res.ok()) return res.error();

    auto root = res.result();

    // This is mostly for context menu commands and hence should be invoked on visible elements.
    // It's not expected that element gets hidden when its format pasted, so the function doesn't
    // change visibility
    bool oldLegendVisible = plot->legend->visible();
    bool oldTitleVisible = plot->title()->visible();
    bool oldAxisVisibleX = plot->xAxis->visible();
    bool oldAxisVisibleY = plot->yAxis->visible();
    QStringList report;
    if (auto err = readLegend(root[KEY_LEGEND].toObject(), plot->legend); err.code == JsonError::BadVersion)
        report << err.message;
    if (auto err = readTitle(root[KEY_TITLE].toObject(), plot->title()); err.code == JsonError::BadVersion)
        report << err.message;
    if (auto err = readAxis(root[KEY_AXIS_X].toObject(), plot->xAxis); err.code == JsonError::BadVersion)
        report << err.message;
    if (auto err = readAxis(root[KEY_AXIS_Y].toObject(), plot->yAxis); err.code == JsonError::BadVersion)
        report << err.message;
    plot->legend->setVisible(oldLegendVisible);
    plot->title()->setVisible(oldTitleVisible);
    plot->xAxis->setVisible(oldAxisVisibleX);
    plot->yAxis->setVisible(oldAxisVisibleY);
    return report.join('\n');
}

QString pasteLegendFormat(QCPLegend* legend)
{
    auto res = getClipboradData(KEY_LEGEND);
    if (!res.ok()) return res.error();

    // This is mostly for context menu commands and hence should be invoked on visible elements.
    // It's not expected that element gets hidden when its format pasted, so the function doesn't
    // change visibility
    bool oldVisible = legend->visible();

    auto err = readLegend(res.result(), legend);
    if (err.code == JsonError::BadVersion)
        return err.message;

    legend->setVisible(oldVisible);
    return {};
}

QString pasteTitleFormat(QCPTextElement* title)
{
    auto res = getClipboradData(KEY_TITLE);
    if (!res.ok()) return res.error();

    // This is mostly for context menu commands and hence should be invoked on visible elements.
    // It's not expected that element gets hidden when its format pasted, so the function doesn't
    // change visibility and there is not need to call `Plot::updateTitleVisibility()` after.
    bool oldVisible = title->visible();

    auto err = readTitle(res.result(), title);
    if (err.code == JsonError::BadVersion)
        return err.message;

    title->setVisible(oldVisible);
    return {};
}

QString pasteAxisFormat(QCPAxis* axis)
{
    auto res = getClipboradData(KEY_AXIS);
    if (!res.ok()) return res.error();

    // This is mostly for context menu commands and hence should be invoked on visible elements.
    // It's not expected that element gets hidden when its format pasted, so the function doesn't
    // change visibility
    bool oldVisible = axis->visible();

    auto err = readAxis(res.result(), axis);
    if (err.code == JsonError::BadVersion)
        return err.message;

    axis->setVisible(oldVisible);
    return {};
}

} // namespace QCPL
