#include "qcpl_io_json.h"

#include "qcpl_utils.h"
#include "qcustomplot/qcustomplot.h"

#include "core/OriResult.h"
#include "tools/OriSettings.h"

#define CURRENT_LEGEND_VERSION 1

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

QJsonObject writePlot(QCustomPlot* plot)
{
    return QJsonObject({
        { "legend", writeLegend(plot->legend) },
    });
}

void readPlot(const QJsonObject& root, QCustomPlot* plot, JsonReport *report)
{
    {
        auto err = readLegend(root["legend"].toObject(), plot->legend);
        if (report and !err.ok()) report->append(err);
    }
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
    QColor backColor(obj["back_color"].toString());
    if (backColor.isValid())
        legend->setBrush(backColor);
    QColor textColor(obj["text_color"].toString());
    if (textColor.isValid())
        legend->setTextColor(textColor);
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

//------------------------------------------------------------------------------
//                          QCPL::FormatStorageIni
//------------------------------------------------------------------------------

static QString jsonToStr(const QJsonObject& obj)
{
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

static QJsonObject varToJson(const QVariant& data)
{
    QString str = data.toString();
    if (str.isEmpty()) return QJsonObject();
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    return doc.isNull() ? QJsonObject() : doc.object();
}

void FormatStorageIni::load(QCustomPlot* plot, JsonReport* report)
{
    Ori::Settings s;
    s.beginGroup("DefaultPlotFormat");
    {
        auto err = QCPL::readLegend(varToJson(s.value("legend")), plot->legend);
        if (report and !err.ok()) report->append(err);
    }
}

void FormatStorageIni::saveLegend(QCPLegend* legend)
{
    Ori::Settings s;
    s.beginGroup("DefaultPlotFormat");
    s.setValue("legend", jsonToStr(QCPL::writeLegend(legend)));
}

void FormatStorageIni::saveTitle(QCPTextElement* title)
{
    // TODO
}

//------------------------------------------------------------------------------
//                              Load / Save
//------------------------------------------------------------------------------

QString loadFormatFromFile(const QString& fileName, QCustomPlot* plot, JsonReport *report)
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

QString saveFormatToFile(const QString& fileName, QCustomPlot* plot)
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

QString pasteLegendFormat(QCPLegend* legend)
{
    auto res = getClipboradData("legend");
    if (!res.ok()) return res.error();

    auto err = readLegend(res.result(), legend);
    if (err.code == JsonError::BadVersion)
        return err.message;

    return {};
}

} // namespace QCPL
