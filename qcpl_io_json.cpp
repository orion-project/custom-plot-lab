#include "qcpl_io_json.h"

#include "qcpl_plot.h"
#include "qcpl_utils.h"

#include "core/OriResult.h"
#include "tools/OriSettings.h"

#define CURRENT_LEGEND_VERSION 1
#define CURRENT_TITLE_VERSION 1

namespace QCPL {

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

QJsonObject writePlot(Plot* plot, const JsonOptions& opts)
{
    return QJsonObject({
        { "legend", writeLegend(plot->legend) },
        { "title", writeTitle(plot->title(), opts) },
    });
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
    if (opts.saveTextContent)
        obj["text"] = title->text();
    return obj;
}

void readPlot(const QJsonObject& root, Plot *plot, JsonReport *report)
{
    {
        auto err = readLegend(root["legend"].toObject(), plot->legend);
        if (report and !err.ok()) report->append(err);
    }
    {
        auto err = readTitle(root["title"].toObject(), plot->title());
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
    if (obj.contains("text"))
        title->setText(obj["text"].toString(title->text()));
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

void FormatStorageIni::load(Plot *plot, JsonReport* report)
{
    Ori::Settings s;
    s.beginGroup("DefaultPlotFormat");
    // Non existent settings keys can be safely read too, they result in empty json objects
    // and read functons should skip empty objects without substituting default values for every prop.
    {
        auto err = readLegend(varToJson(s.value("legend")), plot->legend);
        if (report and !err.ok()) report->append(err);
    }
    {
        auto err = readTitle(varToJson(s.value("title")), plot->title());
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
    opts.saveTextContent = false;
    savePlotFormatIni("title", writeTitle(title, opts));
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
    JsonOptions opts;
    opts.saveTextContent = false;
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
    opts.saveTextContent = false;
    setClipboardData(writeTitle(title, opts), "title");
}

QString pasteLegendFormat(QCPLegend* legend)
{
    auto res = getClipboradData("legend");
    if (!res.ok()) return res.error();

    auto err = readLegend(res.result(), legend);
    if (err.code == JsonError::BadVersion)
        return err.message;

    return {};
}

QString pasteTitleFormat(QCPTextElement* title)
{
    auto res = getClipboradData("title");
    if (!res.ok()) return res.error();

    bool oldVisible = title->visible();
    auto err = readTitle(res.result(), title);
    if (err.code == JsonError::BadVersion)
        return err.message;

    title->setVisible(oldVisible);
    return {};
}

} // namespace QCPL
