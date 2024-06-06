#include "qcpl_colors.h"

#include <QMap>
#include <QPalette>
#include <QRandomGenerator>

namespace QCPL {

// Color set proposed for sequentially added graphs, based on X11 color set;
// Lightmost colors have been eliminated, supposing plot on a white background.
// First colors up to black have been chosen to make graphs easy distinguishing,
// rest just have been slightly sorted to increase distance between close hues.

QVector<QColor> __x11_colors {
    QColor(0x0000FF), // Blue
    QColor(0xFF0000), // Red
    QColor(0x228B22), // Forest Green
    QColor(0xFF8C00), // Dark Orange
    QColor(0x00CED1), // Dark Turquoise
    QColor(0x9400D3), // Dark Violet
    QColor(0x808000), // Olive
    QColor(0x4B0082), // Indigo
    QColor(0xFF69B4), // Hot Pink
    QColor(0x00FF00), // Lime

    QColor(Qt::black), // Black

    QColor(0xFF00FF), // Magenta
    QColor(0x00FF00), // Green (X11)
    QColor(0xDC143C), // Crimson
    QColor(0xA52A2A), // Brown
    QColor(0xD2691E), // Chocolate
    QColor(0x8B008B), // Dark Magenta
    QColor(0xFF7F50), // Coral
    QColor(0x6495ED), // Cornflower
    QColor(0x00008B), // Dark Blue
    QColor(0x008B8B), // Dark Cyan
    QColor(0xB8860B), // Dark Goldenrod
    QColor(0xA9A9A9), // Dark Gray
    QColor(0x8A2BE2), // Blue Violet
    QColor(0x5F9EA0), // Cadet Blue
    QColor(0x7FFF00), // Chartreuse
    QColor(0x006400), // Dark Green
    QColor(0xBDB76B), // Dark Khaki
    QColor(0x556B2F), // Dark Olive Green
    QColor(0x9932CC), // Dark Orchid
    QColor(0x8B0000), // Dark Red
    QColor(0xE9967A), // Dark Salmon
    QColor(0x8FBC8F), // Dark Sea Green
    QColor(0x483D8B), // Dark Slate Blue
    QColor(0x2F4F4F), // Dark Slate Gray
    QColor(0xFF1493), // Deep Pink
    QColor(0x00BFFF), // Deep Sky Blue
    QColor(0xFF4500), // Orange Red
    QColor(0x696969), // Dim Gray
    QColor(0x1E90FF), // Dodger Blue
    QColor(0xB22222), // Firebrick
    QColor(0xFF00FF), // Fuchsia
    QColor(0xFFD700), // Gold
    QColor(0xDAA520), // Goldenrod
    QColor(0xBEBEBE), // Gray (X11)
    QColor(0x7F7F7F), // Gray (W3C)
    QColor(0x007F00), // Green (W3C)
    QColor(0xADFF2F), // Green Yellow
    QColor(0xCD5C5C), // Indian Red
    QColor(0xF0E68C), // Khaki
    QColor(0x7CFC00), // Lawn Green
    QColor(0x32CD32), // Lime Green
    QColor(0xFAF0E6), // Linen
    QColor(0xB03060), // Maroon (X11)
    QColor(0x0000CD), // Medium Blue
    QColor(0x66CDAA), // Medium Aquamarine
    QColor(0xBA55D3), // Medium Orchid
    QColor(0x3CB371), // Medium Sea Green
    QColor(0x7B68EE), // Medium Slate Blue
    QColor(0x00FA9A), // Medium Spring Green
    QColor(0x48D1CC), // Medium Turquoise
    QColor(0xC71585), // Medium Violet Red
    QColor(0x191970), // Midnight Blue
    QColor(0x000080), // Navy
    QColor(0xFFA500), // Orange
    QColor(0x7F0000), // Maroon (W3C)
    QColor(0xDA70D6), // Orchid
    QColor(0xDB7093), // Pale Violet Red
    QColor(0x9370DB), // Medium Purple
    QColor(0xCD853F), // Peru
    QColor(0xB0E0E6), // Powder Blue
    QColor(0xA020F0), // Purple (X11)
    QColor(0x7F007F), // Purple (W3C)
    QColor(0xBC8F8F), // Rosy Brown
    QColor(0x6B8E23), // Olive Drab
    QColor(0x4169E1), // Royal Blue
    QColor(0x8B4513), // Saddle Brown
    QColor(0xFA8072), // Salmon
    QColor(0xF4A460), // Sandy Brown
    QColor(0x2E8B57), // Sea Green
    QColor(0xA0522D), // Sienna
    QColor(0xC0C0C0), // Silver
    QColor(0x87CEEB), // Sky Blue
    QColor(0x6A5ACD), // Slate Blue
    QColor(0x708090), // Slate Gray
    QColor(0x00FF7F), // Spring Green
    QColor(0x4682B4), // Steel Blue
    QColor(0xD2B48C), // Tan
    QColor(0x008080), // Teal
    QColor(0xD8BFD8), // Thistle
    QColor(0xFF6347), // Tomato
    QColor(0x40E0D0), // Turquoise
    QColor(0xEE82EE), // Violet
    QColor(0xF5DEB3), // Wheat
    QColor(0xFFFF00), // Yellow
    QColor(0x9ACD32), // Yellow Green
    QColor(0xADD8E6), // Light Blue
    QColor(0xF08080), // Light Coral
    QColor(0xE0FFFF), // Light Cyan
    QColor(0xFAFAD2), // Light Goldenrod
    QColor(0xD3D3D3), // Light Gray
    QColor(0x90EE90), // Light Green
    QColor(0xFFB6C1), // Light Pink
    QColor(0xFFA07A), // Light Salmon
    QColor(0xFFE4B5), // Moccasin
    QColor(0x20B2AA), // Light Sea Green
    QColor(0x87CEFA), // Light Sky Blue
    QColor(0x778899), // Light Slate Gray
    QColor(0xB0C4DE), // Light Steel Blue
    QColor(0xFFFFE0), // Light Yellow
    QColor(0xFFDAB9), // Peach Puff
    QColor(0xFFC0CB), // Pink
    QColor(0xDDA0DD), // Plum
    QColor(0x00FFFF), // Aqua
    QColor(0x7FFFD4), // Aquamarine
    QColor(0xFAEBD7), // Antique White
    QColor(0xF5F5DC), // Beige
    QColor(0xFFE4C4), // Bisque
    QColor(0xFFEBCD), // Blanched Almond
    QColor(0x00FFFF), // Cyan
    QColor(0xDEB887), // Burlywood
    QColor(0xFFF8DC), // Cornsilk
    QColor(0xFFEFD5), // Papaya Whip
    QColor(0xDCDCDC), // Gainsboro
    QColor(0xF0FFF0), // Honeydew
    QColor(0xFFFACD), // Lemon Chiffon
    QColor(0xFFE4E1), // Misty Rose
    QColor(0xFFDEAD), // Navajo White
    QColor(0xEEE8AA), // Pale Goldenrod
    QColor(0x98FB98), // Pale Green
    QColor(0xAFEEEE), // Pale Turquoise

    // These are too light to use as grapg line colors
    //QColor(0xF0F8FF), // Alice Blue
    //QColor(0xF0FFFF), // Azure
    //QColor(0xFFFAF0), // Floral White
    //QColor(0xF8F8FF), // Ghost White
    //QColor(0xFFFFF0), // Ivory
    //QColor(0xE6E6FA), // Lavender
    //QColor(0xFFF0F5), // Lavender Blush
    //QColor(0xF5FFFA), // Mint Cream
    //QColor(0xFDF5E6), // Old Lace
    //QColor(0xFFF5EE), // Seashell
    //QColor(0xFFFAFA), // Snow
    //QColor(0xFFFFFF), // White
    //QColor(0xF5F5F5), // White Smoke
};

const QVector<QColor>& defaultColorSet()
{
    return __x11_colors;
}

QColor getRandomColor()
{
    return __x11_colors.at(QRandomGenerator::global()->generate() % __x11_colors.size());
}

typedef QMap<GuiColor, QColor> GuiColorMap;
Q_GLOBAL_STATIC(GuiColorMap, __colorMap);

QColor guiColor(GuiColor kind)
{
    if (__colorMap->contains(kind))
        return __colorMap->value(kind);
    return QPalette().mid().color().name(QColor::HexRgb);
}

void setGuiColors(const QMap<GuiColor, QColor> &colors)
{
    for (auto it = colors.constBegin(); it != colors.constEnd(); it++)
        __colorMap->insert(it.key(), it.value());
}

} // namespace QCPL
