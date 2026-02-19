#include "qcpl_io_json.h"

#include "qcpl_plot.h"
#include "qcpl_utils.h"

#include "core/OriResult.h"
#include "tools/OriSettings.h"

#define MIME_TYPE "application/x-orion-project-org;value=plot-format"
#define CURRENT_LEGEND_VERSION 1
#define CURRENT_TITLE_VERSION 1
#define CURRENT_AXIS_VERSION 1
#define CURRENT_GRAPH_VERSION 1
#define SECTION_INI "DefaultPlotFormat"
#define KEY_TITLE "title"
#define KEY_LEGEND "legend"
#define KEY_AXIS "axis"
#define KEY_AXIS_X "axis_x"
#define KEY_AXIS_Y "axis_y"
#define KEY_GRAPH "graph"

namespace QCPL {

namespace {

QJsonValue colorToJson(const QColor& color)
{
    return color.name();
}

QColor jsonToColor(const QJsonValue& val, const QColor& def)
{
    QColor color(val.toString());
    return color.isValid() ? color : def;
}

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

QJsonObject writeGradient(const QCPColorGradient& grad)
{
    auto obj = QJsonObject({
        { "level_count", grad.levelCount() },
        { "color_interpolation", int(grad.colorInterpolation()) },
        { "nan_handling", int(grad.nanHandling()) },
        { "nan_color", colorToJson(grad.nanColor()) },
        { "periodic", grad.periodic() },
    });
    QJsonArray jsonStops;
    auto stops = grad.colorStops();
    for (auto it = stops.constBegin(); it != stops.constEnd(); it++)
    {
        jsonStops.append(QJsonObject({
            { "stop", it.key() },
            { "color", colorToJson(it.value()) },
        }));
    }
    obj["color_stops"] = jsonStops;
    return obj;
}

QCPColorGradient readGradient(const QJsonObject& obj, const QCPColorGradient& def)
{
    QCPColorGradient grad;
    grad.setLevelCount(obj["level_count"].toInt(def.levelCount()));
    grad.setColorInterpolation(QCPColorGradient::ColorInterpolation(obj["color_interpolation"].toInt(int(def.colorInterpolation()))));
    grad.setNanHandling(QCPColorGradient::NanHandling(obj["nan_handling"].toInt(int(def.nanHandling()))));
    grad.setNanColor(jsonToColor(obj["nan_color"], def.nanColor()));
    grad.setPeriodic(obj["periodic"].toBool(def.periodic()));
    grad.clearColorStops();
    QJsonArray jsonStops = obj["color_stops"].toArray();
    for (auto it = jsonStops.constBegin(); it != jsonStops.constEnd(); it++)
    {
        auto jsonStop = (*it).toObject();
        QColor color(jsonStop["color"].toString());
        if (color.isValid())
            grad.setColorStopAt(jsonStop["stop"].toDouble(), color);
    }
    return grad;
}

} // namespace

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

//------------------------------------------------------------------------------
//                             Write to JSON

static QString makeAxisKey(const QString base, int index) {
    return index == 0 ? base : QString("%1_%2").arg(base).arg(index);
}

QJsonObject writePlot(Plot* plot, const WritePlotOptions &opts)
{
    auto titleJson = writeTitle(plot->title());
    if (opts.titleText)
        titleJson["formatter_text"] = plot->formatterText(plot->title());

    QJsonObject root({
        { KEY_LEGEND, writeLegend(plot->legend) },
        { KEY_TITLE, titleJson },
    });

    QMap<QString, QCPAxis*> saveAxes;
    auto axes = plot->axisRect()->axes(QCPAxis::atBottom);
    for (int i = 0; i < axes.size(); i++) {
        saveAxes.insert(makeAxisKey("axis_x", i), axes.at(i));
        if (opts.onlyPrimaryAxes) break;
    }
    axes = plot->axisRect()->axes(QCPAxis::atLeft);
    for (int i = 0; i < axes.size(); i++) {
        saveAxes.insert(makeAxisKey("axis_y", i), axes.at(i));
        if (opts.onlyPrimaryAxes) break;
    }
    if (!opts.onlyPrimaryAxes) {
        axes = plot->axisRect()->axes(QCPAxis::atTop);
        for (int i = 0; i < axes.size(); i++)
            saveAxes.insert(makeAxisKey("axis_x2", i), axes.at(i));
        axes = plot->axisRect()->axes(QCPAxis::atRight);
        for (int i = 0; i < axes.size(); i++)
            saveAxes.insert(makeAxisKey("axis_y2", i), axes.at(i));
    }
    for (auto it = saveAxes.cbegin(); it != saveAxes.cend(); it++) {
        auto axis = it.value();
        auto axisJson = writeAxis(axis, opts.axesTexts, opts.axesLimits);
        auto id = axis->objectName();
        if (!id.isEmpty())
            axisJson["id"] = id;
        if (opts.axesTexts)
            axisJson["formatter_text"] = plot->formatterText(axis);
        if (opts.axesLimits) {
            auto factor = plot->axisFactor(axis);
            if (std::holds_alternative<int>(factor))
                axisJson["factor"] = std::get<int>(factor);
            else axisJson["factor_custom"] = std::get<double>(factor);
        }
        root[it.key()] = axisJson;
    }

    for (auto it = plot->additionalParts.cbegin(); it != plot->additionalParts.cend(); it++)
    {
        if (auto colorScale = qobject_cast<QCPColorScale*>(it.key()); colorScale)
        {
            root[it.value()] = writeColorScale(colorScale);
            continue;
        }
        qWarning() << "writePlot: Unknown how lo write object to key" << it.value();
    }
    return root;
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

QJsonObject writeTitle(QCPTextElement* title, bool andText)
{
    auto obj = QJsonObject({
        { "version", CURRENT_TITLE_VERSION },
        { "visible", title->visible() },
        { "font", writeFont(title->font()) },
        { "text_color", colorToJson(title->textColor()) },
        { "text_flags", title->textFlags() },
        { "margins", writeMargins(title->margins()) },
    });
    if (andText)
        obj["text"] = title->text();
    return obj;
}

QJsonObject writeAxis(QCPAxis *axis, bool andText, bool andLimits)
{
    auto grid = axis->grid();
    auto ticker = axis->ticker();
    auto obj = QJsonObject({
        { "version", CURRENT_AXIS_VERSION },
        { "id", axis->objectName() },
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
        { "tick_strategy", int(ticker->tickStepStrategy()) },
        { "tick_count", ticker->tickCount() },
        { "tick_offset", ticker->tickOrigin() },
    });
    if (andText)
        obj["text"] = axis->label();
    if (andLimits) {
        auto range = axis->range();
        obj["range_min"] = range.lower;
        obj["range_max"] = range.upper;
    }
    return obj;
}

QJsonObject writeColorScale(QCPColorScale *scale)
{
    auto obj = writeAxis(scale->axis());
    obj["color_bar_width"] = scale->barWidth();
    obj["color_bar_gradient"] = writeGradient(scale->gradient());
    obj["color_bar_margins"] = writeMargins(scale->margins());
    return obj;
}

QJsonObject writeGraph(QCPGraph * graph)
{
    auto scatter = graph->scatterStyle();
    return QJsonObject({
        { "version", CURRENT_GRAPH_VERSION },
        { "line_pen", writePen(graph->pen()) },
        { "scatter_pen", writePen(scatter.pen()) },
        { "scatter_color", colorToJson(scatter.brush().color()) },
        { "scatter_shape", int(scatter.shape()) },
        { "scatter_size", scatter.size() },
        { "scatter_skip", graph->scatterSkip() },
    });
}

//------------------------------------------------------------------------------
//                             Read from JSON

void readPlot(const QJsonObject& root, Plot *plot, JsonReport *report, const ReadPlotOptions& opts)
{
    if (auto err = readLegend(root[KEY_LEGEND].toObject(), plot->legend); !err.ok() && report)
        report->append(err);

    auto titleJson = root[KEY_TITLE].toObject();
    if (auto err = readTitle(titleJson, plot->title(), opts.titleText); !err.ok() && report)
        report->append(err);
    if (opts.titleText && titleJson.contains("formatter_text"))
        plot->setFormatterText(plot->title(), titleJson["formatter_text"].toString());

    const QLatin1String axisKeyPrefix("axis_");
    QList<QPair<int, QString>> bottomAxisKeys, leftAxisKeys, topAxisKeys, rightAxisKeys;
    foreach (const auto& key, root.keys())
    {
        if (!key.startsWith(axisKeyPrefix)) continue;
        auto s = QStringView(key).right(key.size() - axisKeyPrefix.size());
        QList<QPair<int, QString>> *keys;
        int indexOffset;
        if (s.startsWith(QLatin1String("y2"))) keys = &rightAxisKeys, indexOffset = 2;
        else if (s.startsWith(QLatin1String("x2"))) keys = &topAxisKeys, indexOffset = 2;
        else if (s.startsWith('y')) keys = &leftAxisKeys, indexOffset = 1;
        else if (s.startsWith('x')) keys = &bottomAxisKeys, indexOffset = 1;
        else continue;
        s = s.right(s.size() - indexOffset);
        int index = s.startsWith('_') ? s.right(s.size()-1).toInt() : 0;
        keys->append({index, key});
    }
    auto readAxes = [root, plot, report, opts](QList<QPair<int, QString>>& keys, QCPAxis::AxisType axisType) {
        std::sort(keys.begin(), keys.end(), [](const QPair<int, QString>& a, const QPair<int, QString>&b){
            return a.first < b.first;
        });
        auto axes = plot->axisRect()->axes(axisType);
        for (int i = 0; i < keys.size(); i++)
        {
            auto key = keys.at(i).second;
            if (i < axes.size()) {
                // pass
            } else if (opts.autoCreateAxes) {
                axes << plot->addAxis(axisType);
            } else break;
            auto axis = axes.at(i);
            auto axisJson = root[key].toObject();
            if (auto err = readAxis(axisJson, axis, opts.axesTexts, opts.axesLimits); !err.ok() && report)
                report->append(err);
            if (opts.axesTexts && axisJson.contains("formatter_text"))
                if (plot->ensureFormatter(axis))
                    plot->setFormatterText(axis, axisJson["formatter_text"].toString());
            if (opts.axesLimits && (axisJson.contains("factor") || axisJson.contains("factor_custom"))) {
                if (axisJson.contains("factor"))
                    plot->setAxisFactor(axis, axisJson["factor"].toInt());
                else plot->setAxisFactor(axis, axisJson["factor_custom"].toDouble());
            }
            if (axisJson.contains("id"))
                axis->setObjectName(axisJson["id"].toString());
        }
    };
    readAxes(bottomAxisKeys, QCPAxis::atBottom);
    readAxes(leftAxisKeys, QCPAxis::atLeft);
    readAxes(topAxisKeys, QCPAxis::atTop);
    readAxes(rightAxisKeys, QCPAxis::atRight);

    for (auto it = plot->additionalParts.constBegin(); it != plot->additionalParts.constEnd(); it++)
    {
        if (auto colorScale = qobject_cast<QCPColorScale*>(it.key()); colorScale)
        {
            if (auto err = readColorScale(root[it.value()].toObject(), colorScale); !err.ok() && report)
                report->append(err);
            continue;
        }
        qWarning() << "readPlot: Unknown how to read object from key" << it.value();
    }
    plot->updateTitleVisibility();
    plot->updateAxesInteractivity();
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

JsonError readTitle(const QJsonObject &obj, QCPTextElement* title, bool andText)
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
    if (andText && obj.contains("text"))
        title->setText(obj["text"].toString());
    return {};
}

JsonError readAxis(const QJsonObject &obj, QCPAxis* axis, bool andText, bool andLimits)
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
    auto ticker = axis->ticker();
    ticker->setTickStepStrategy(QCPAxisTicker::TickStepStrategy(obj["tick_strategy"].toInt(int(ticker->tickStepStrategy()))));
    ticker->setTickCount(obj["tick_count"].toInt(ticker->tickCount()));
    ticker->setTickOrigin(obj["tick_offset"].toDouble(ticker->tickOrigin()));
    updateAxisTicker(axis);
    if (andText && obj.contains("text"))
        axis->setLabel(obj["text"].toString());
    if (andText && obj.contains("range_min") && obj.contains("range_max"))
        axis->setRange(QCPRange(obj["range_min"].toDouble(), obj["range_max"].toDouble()));
    return {};
}

JsonError readColorScale(const QJsonObject &obj, QCPColorScale *scale)
{
    auto err = readAxis(obj, scale->axis());
    if (!err.ok())
        return err;
    scale->setBarWidth(obj["color_bar_width"].toInt(scale->barWidth()));
    scale->setGradient(readGradient(obj["color_bar_gradient"].toObject(), scale->gradient()));
    scale->setMargins(readMargins(obj["color_bar_margins"].toObject(), scale->margins()));
    return {};
}

JsonError readGraph(const QJsonObject &obj, QCPGraph * graph)
{
    if (obj.isEmpty())
        return { JsonError::NoData, "Line format object is empty" };
    auto ver = obj["version"].toInt();
    if (ver != CURRENT_GRAPH_VERSION)
        return {
                JsonError::BadVersion,
                QString("Unsupported line format version %1, expected %2").arg(ver, CURRENT_GRAPH_VERSION) };
    graph->setPen(readPen(obj["line_pen"].toObject(), graph->pen()));
    QCPScatterStyle scatter = graph->scatterStyle();
    scatter.setPen(readPen(obj["scatter_pen"].toObject(), scatter.pen()));
    scatter.setBrush(jsonToColor(obj["scatter_color"], scatter.brush().color()));
    scatter.setShape(QCPScatterStyle::ScatterShape(obj["scatter_shape"].toInt(int(scatter.shape()))));
    scatter.setSize(obj["scatter_size"].toDouble(scatter.size()));
    graph->setScatterStyle(scatter);
    graph->setScatterSkip(obj["scatter_skip"].toInt(graph->scatterSkip()));
    return {};
}

//------------------------------------------------------------------------------
//                          QCPL::FormatStorageIni
//------------------------------------------------------------------------------

QString findStorageKey(const char* func, QCPLayerable* obj)
{
    auto plot = qobject_cast<Plot*>(obj->parentPlot());
    if (!plot || !plot->additionalParts.contains(obj))
    {
        qWarning() << func << "Object is not registerd in parent plot as storable object";
        return {};
    }
    return plot->additionalParts[obj];
}

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
    for (auto it = plot->additionalParts.constBegin(); it != plot->additionalParts.constEnd(); it++)
    {
        if (auto colorScale = qobject_cast<QCPColorScale*>(it.key()); colorScale)
        {
            s.setValue(it.value(), jsonToVar(writeColorScale(colorScale)));
            continue;
        }
        qWarning() << "FormatStorageIni::save: Unknown how to save object with key" << it.value();
    }
}

void FormatStorageIni::load(Plot *plot, JsonReport* report)
{
    Ori::Settings s;
    s.beginGroup(SECTION_INI);
    // Non existent settings keys can be safely read too, they result in empty json objects
    // and read functons should skip empty objects without substituting default values for every prop.
    if (auto err = readLegend(varToJson(s.value(KEY_LEGEND)), plot->legend); !err.ok() && report)
        report->append(err);
    if (auto err = readTitle(varToJson(s.value(KEY_TITLE)), plot->title()); !err.ok() && report)
            report->append(err);
    if (auto err = readAxis(varToJson(s.value(KEY_AXIS_X)), plot->xAxis); !err.ok() && report)
        report->append(err);
    if (auto err = readAxis(varToJson(s.value(KEY_AXIS_Y)), plot->yAxis); !err.ok() && report)
        report->append(err);
    for (auto it = plot->additionalParts.constBegin(); it != plot->additionalParts.constEnd(); it++)
    {
        if (auto colorScale = qobject_cast<QCPColorScale*>(it.key()); colorScale)
        {
            if (auto err = readColorScale(varToJson(s.value(it.value())), colorScale); !err.ok() && report)
                report->append(err);
            continue;
        }
        qWarning() << "FormatStorageIni::load: Unknown how to read object from key" << it.value();
    }
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

void FormatStorageIni::saveColorScale(QCPColorScale* scale)
{
    if (auto key = findStorageKey("FormatStorageIni::saveColorScale", scale); !key.isEmpty())
        savePlotFormatIni(key, writeColorScale(scale));
}

//------------------------------------------------------------------------------
//                              Load / Save
//------------------------------------------------------------------------------

QString loadFormatFromFile(const QString& fileName, Plot* plot, JsonReport *report, const ReadPlotOptions& opts)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "Unable to open file for reading: " + file.errorString();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (doc.isNull())
        return "Unable to parse json file: " + error.errorString();

    readPlot(doc.object(), plot, report, opts);
    return {};
}

QString saveFormatToFile(const QString& fileName, Plot* plot, const WritePlotOptions& opts)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return "Unable to open file for writing: " + file.errorString();
    QTextStream(&file) << QJsonDocument(writePlot(plot, opts)).toJson();
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

void copyColorScaleFormat(QCPColorScale* scale)
{
    if (auto key = findStorageKey("copyColorScaleFormat", scale); !key.isEmpty())
        setClipboardData(writeColorScale(scale), key);
}

void copyGraphFormat(QCPGraph* graph)
{
    setClipboardData(writeGraph(graph), KEY_GRAPH);
}

QString pastePlotFormat(Plot* plot)
{
    auto res = getClipboradData("plot");
    if (!res.ok()) return res.error();
    auto root = res.result();
    // This is mostly for context menu commands and hence should be invoked on visible elements.
    // It's not expected that element gets hidden when its format pasted, so the function doesn't
    // change visibility
    QHash<QCPLayerable*, bool> oldVisibility {
        { plot->title(), plot->title()->visible() },
        { plot->legend, plot->legend->visible() },
        { plot->xAxis, plot->xAxis->visible() },
        { plot->yAxis, plot->yAxis->visible() },
    };
    for (auto it = plot->additionalParts.constBegin(); it != plot->additionalParts.constEnd(); it++)
        oldVisibility[it.key()] = it.key()->visible();
    JsonReport report;
    readPlot(root, plot, &report);
    QStringList strReport;
    for (auto& err : report)
        if (!err.ok() && err.code != JsonError::NoData)
            strReport << err.message;
    for (auto it = oldVisibility.constBegin(); it != oldVisibility.constEnd(); it++)
        it.key()->setVisible(it.value());
    plot->updateTitleVisibility();
    return strReport.join('\n');
}

QString pasteLegendFormat(QCPLegend* legend)
{
    auto res = getClipboradData(KEY_LEGEND);
    if (!res.ok()) return res.error();

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

    bool oldVisible = axis->visible();

    auto err = readAxis(res.result(), axis);
    if (err.code == JsonError::BadVersion)
        return err.message;

    axis->setVisible(oldVisible);
    return {};
}

QString pasteColorScaleFormat(QCPColorScale* scale)
{
    auto key = findStorageKey("copyColorScaleFormat", scale);
    if (key.isEmpty())
        return "Operation is not supported";

    auto res = getClipboradData(key);
    if (!res.ok()) return res.error();

    bool oldVisible = scale->visible();

    auto err = readColorScale(res.result(), scale);
    if (err.code == JsonError::BadVersion)
        return err.message;

    scale->setVisible(oldVisible);
    return {};
}

QString pasteGraphFormat(QCPGraph* graph)
{
    auto res = getClipboradData(KEY_GRAPH);
    if (!res.ok()) return res.error();

    auto err = readGraph(res.result(), graph);
    if (err.code == JsonError::BadVersion)
        return err.message;

    return {};
}

} // namespace QCPL
