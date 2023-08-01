#include "qcpl_utils.h"
#include "qcustomplot/qcustomplot.h"

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

QJsonObject writeSize(const QSize& size)
{
    return QJsonObject({
        { "width", size.width() },
        { "height", size.height() },
    });
}

QJsonObject writeMargins(const QMargins& margins)
{
    return QJsonObject({
        { "top", margins.top() },
        { "right", margins.right() },
        { "bottom", margins.bottom() },
        { "left", margins.left() },
    });
}

void writeLegend(QJsonObject& root, QCPLegend* legend)
{
    root["legend"] = QJsonObject({
        { "version", 1 },
        { "visible", legend->visible() },
        { "back_color", legend->brush().color().name() },
        { "text_color", legend->textColor().name() },
        { "font", writeFont(legend->font()) },
        { "icon_size", writeSize(legend->iconSize()) },
        { "icon_margin", legend->iconTextPadding() },
        { "paddings", writeMargins(legend->margins()) },
        { "margins", writeMargins(legendLayout(legend)->margins()) },
        { "location", int(legendLayout(legend)->insetAlignment(0)) }
    });
}

} // namespace QCPL
