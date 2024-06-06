#ifndef QCPL_COLORS_H
#define QCPL_COLORS_H

#include <QVector>
#include <QColor>

namespace QCPL {

const QVector<QColor>& defaultColorSet();
QColor getRandomColor();

enum GuiColor
{
    HintLabelColor,
};

QColor guiColor(GuiColor kind);
void setGuiColors(const QMap<GuiColor, QColor> &colors);

} // namespace QCPL

#endif // QCPL_COLORS_H
