#include "qcpl_io_json.h"

#include "qcpl_utils.h"
#include "qcustomplot/qcustomplot.h"

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

void writeLegend(QJsonObject& root, QCPLegend* legend)
{
    root["legend"] = QJsonObject({
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

JsonResult readLegend(const QJsonObject& root, QCPLegend* legend)
{
    if (!root.contains("legend"))
        return { JsonResult::KeyNotFound, QString("Key not found: 'legend'") };
    auto obj = root["legend"].toObject();
    return readLegendObj(obj, legend);
}

JsonResult readLegendObj(const QJsonObject& obj, QCPLegend* legend)
{
    auto ver = obj["version"].toInt();
    if (ver != CURRENT_LEGEND_VERSION)
        return { JsonResult::BadVersion,
                QString("Unsupported legend version %1, expected is %2").arg(ver).arg(CURRENT_LEGEND_VERSION) };
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
    return JsonResult();
}

} // namespace QCPL
