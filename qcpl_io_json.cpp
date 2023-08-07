#include "qcpl_io_json.h"

#include "qcpl_plot.h"
#include "qcpl_utils.h"

#include "core/OriResult.h"
#include "tools/OriSettings.h"

#define CURRENT_LEGEND_VERSION 1
#define CURRENT_TITLE_VERSION 1
#define CURRENT_AXIS_VERSION 1

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

QJsonObject writePlot(Plot* plot, const JsonOptions& opts)
{
    return QJsonObject({
        { "legend", writeLegend(plot->legend) },
        { "title", writeTitle(plot->title(), opts) },
        { "axis_x", writeAxis(plot->xAxis, opts) },
        { "axis_y", writeAxis(plot->yAxis, opts) },
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

QJsonObject writeTitle(QCPTextElement* title, const JsonOptions& opts)
{
    auto obj = QJsonObject({
        { "version", CURRENT_TITLE_VERSION },
        { "visible", title->visible() },
        { "font", writeFont(title->font()) },
        { "text_color", colorToJson(title->textColor()) },
        { "text_flags", title->textFlags() },
        { "margins", writeMargins(title->margins()) },
    });
    if (opts.allowTextContent)
        obj["text"] = title->text();
    return obj;
}

QJsonObject writeAxis(QCPAxis *axis, const JsonOptions& opts)
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
    if (opts.allowTextContent)
        obj["title"] = axis->label();
    return obj;
}

//------------------------------------------------------------------------------
//                             Read from JSON

void readPlot(const QJsonObject& root, Plot *plot, const JsonOptions &opts, JsonReport *report)
{
    {
        auto err = readLegend(root["legend"].toObject(), plot->legend);
        if (report and !err.ok()) report->append(err);
    }
    {
        auto err = readTitle(root["title"].toObject(), plot->title(), opts);
        if (report and !err.ok()) report->append(err);
    }
    {
        auto err = readAxis(root["axis_x"].toObject(), plot->xAxis, opts);
        if (report and !err.ok()) report->append(err);
    }
    {
        auto err = readAxis(root["axis_y"].toObject(), plot->yAxis, opts);
        if (report and !err.ok()) report->append(err);
    }
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

JsonError readTitle(const QJsonObject &obj, QCPTextElement* title, const JsonOptions& opts)
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
    if (obj.contains("text") and opts.allowTextContent)
        title->setText(obj["text"].toString(title->text()));
    return {};
}

JsonError readAxis(const QJsonObject &obj, QCPAxis* axis, const JsonOptions& opts)
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
    if (obj.contains("title") and opts.allowTextContent)
        axis->setLabel(obj["title"].toString(axis->label()));
    return {};
}

//------------------------------------------------------------------------------
//                          QCPL::FormatStorageIni
//------------------------------------------------------------------------------

static void savePlotFormatIni(const QString& key, const QJsonObject& obj)
{
    Ori::Settings s;
    s.beginGroup("DefaultPlotFormat");
    s.setValue(key, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

static QJsonObject varToJson(const QVariant& data)
{
    QString str = data.toString();
    if (str.isEmpty()) return QJsonObject();
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    return doc.isNull() ? QJsonObject() : doc.object();
}

static QString makeAxisKey(QCPAxis* axis)
{
    auto plot = axis->parentPlot();
    if (axis == plot->xAxis) return "axis_x";
    if (axis == plot->yAxis) return "axis_y";
    return "axis";
}

void FormatStorageIni::load(Plot *plot, JsonReport* report)
{
    JsonOptions opts;
    opts.allowTextContent = false;

    Ori::Settings s;
    s.beginGroup("DefaultPlotFormat");
    // Non existent settings keys can be safely read too, they result in empty json objects
    // and read functons should skip empty objects without substituting default values for every prop.
    {
        auto err = readLegend(varToJson(s.value("legend")), plot->legend);
        if (report and !err.ok()) report->append(err);
    }
    {
        auto err = readTitle(varToJson(s.value("title")), plot->title(), opts);
        if (report and !err.ok()) report->append(err);
    }
    {
        auto err = readAxis(varToJson(s.value(makeAxisKey(plot->xAxis))), plot->xAxis, opts);
        if (report and !err.ok()) report->append(err);
    }
    {
        auto err = readAxis(varToJson(s.value(makeAxisKey(plot->yAxis))), plot->yAxis, opts);
        if (report and !err.ok()) report->append(err);
    }
    plot->updateTitleVisibility();
}

void FormatStorageIni::saveLegend(QCPLegend* legend)
{
    savePlotFormatIni("legend", writeLegend(legend));
}

void FormatStorageIni::saveTitle(QCPTextElement* title)
{
    JsonOptions opts;
    opts.allowTextContent = false;
    savePlotFormatIni("title", writeTitle(title, opts));
}

void FormatStorageIni::saveAxis(QCPAxis* axis)
{
    JsonOptions opts;
    opts.allowTextContent = false;
    savePlotFormatIni(makeAxisKey(axis), writeAxis(axis, opts));
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

    JsonOptions opts;
    opts.allowTextContent = false;
    readPlot(doc.object(), plot, opts, report);
    return {};
}

QString saveFormatToFile(const QString& fileName, Plot* plot)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return "Unable to open file for writing: " + file.errorString();
    JsonOptions opts;
    opts.allowTextContent = false;
    QTextStream(&file) << QJsonDocument(writePlot(plot, opts)).toJson();
    return QString();
}

//------------------------------------------------------------------------------
//                          Copy / Paste
//------------------------------------------------------------------------------

static void setClipboardData(const QJsonObject& value, const QString& dataType)
{
    QJsonObject root({{ dataType, value }});
    qApp->clipboard()->setText(QJsonDocument(root).toJson());
}

using JsonResult = Ori::Result<QJsonObject>;

static JsonResult getClipboradData(const QString& dataType)
{
    QString text = qApp->clipboard()->text();
    if (text.isEmpty())
        return JsonResult::fail("Clipboard is empty");

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
    if (doc.isNull())
        return JsonResult::fail("Clipboard text is not a valid JSON text: " + error.errorString());

    QJsonObject root = doc.object();
    if (!root.contains(dataType))
        return JsonResult::fail("There is no data of appropriate type in Clipboard");

    return JsonResult::ok(root[dataType].toObject());
}

void copyLegendFormat(QCPLegend* legend)
{
    setClipboardData(writeLegend(legend), "legend");
}

void copyTitleFormat(QCPTextElement* title)
{
    JsonOptions opts;
    opts.allowTextContent = false;
    setClipboardData(writeTitle(title, opts), "title");
}

void copyAxisFormat(QCPAxis* axis)
{
    JsonOptions opts;
    opts.allowTextContent = false;
    setClipboardData(writeAxis(axis, opts), "axis");
}

QString pasteLegendFormat(QCPLegend* legend)
{
    auto res = getClipboradData("legend");
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
    auto res = getClipboradData("title");
    if (!res.ok()) return res.error();

    // This is mostly for context menu commands and hence should be invoked on visible elements.
    // It's not expected that element gets hidden when its format pasted, so the function doesn't
    // change visibility and there is not need to call `Plot::updateTitleVisibility()` after.
    bool oldVisible = title->visible();

    JsonOptions opts;
    opts.allowTextContent = false;
    auto err = readTitle(res.result(), title, opts);
    if (err.code == JsonError::BadVersion)
        return err.message;

    title->setVisible(oldVisible);
    return {};
}

QString pasteAxisFormat(QCPAxis* axis)
{
    auto res = getClipboradData("axis");
    if (!res.ok()) return res.error();

    // This is mostly for context menu commands and hence should be invoked on visible elements.
    // It's not expected that element gets hidden when its format pasted, so the function doesn't
    // change visibility
    bool oldVisible = axis->visible();

    JsonOptions opts;
    opts.allowTextContent = false;
    auto err = readAxis(res.result(), axis, opts);
    if (err.code == JsonError::BadVersion)
        return err.message;

    axis->setVisible(oldVisible);
    return {};
}

} // namespace QCPL
