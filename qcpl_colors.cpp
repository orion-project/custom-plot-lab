#include "qcpl_colors.h"

namespace QCPL {

// Color set proposed for sequentially added graphs, based on X11 color set;
// Lightmost colors have been eliminated, supposing plot on a white background.
// First colors up to black have been chosen to make graphs easy distinguishing,
// rest just have been slightly sorted to increase distance between close hues.

QVector<QColor> __x11_colors {
            QColor("#0000FF"), // Blue
            QColor("#FF0000"), // Red
            QColor("#228B22"), // Forest Green
            QColor("#FF8C00"), // Dark Orange
            QColor("#00CED1"), // Dark Turquoise
            QColor("#9400D3"), // Dark Violet
            QColor("#808000"), // Olive
            QColor("#4B0082"), // Indigo
            QColor("#FF69B4"), // Hot Pink
            QColor("#00FF00"), // Lime

            QColor("#000000"), // Black

            QColor("#FF00FF"), // Magenta
            QColor("#00FF00"), // Green (X11)
            QColor("#DC143C"), // Crimson
            QColor("#A52A2A"), // Brown
            QColor("#D2691E"), // Chocolate
            QColor("#8B008B"), // Dark Magenta
            QColor("#FF7F50"), // Coral
            QColor("#6495ED"), // Cornflower
            QColor("#00008B"), // Dark Blue
            QColor("#008B8B"), // Dark Cyan
            QColor("#B8860B"), // Dark Goldenrod
            QColor("#A9A9A9"), // Dark Gray
            QColor("#8A2BE2"), // Blue Violet
            QColor("#5F9EA0"), // Cadet Blue
            QColor("#7FFF00"), // Chartreuse
            QColor("#006400"), // Dark Green
            QColor("#BDB76B"), // Dark Khaki
            QColor("#556B2F"), // Dark Olive Green
            QColor("#9932CC"), // Dark Orchid
            QColor("#8B0000"), // Dark Red
            QColor("#E9967A"), // Dark Salmon
            QColor("#8FBC8F"), // Dark Sea Green
            QColor("#483D8B"), // Dark Slate Blue
            QColor("#2F4F4F"), // Dark Slate Gray
            QColor("#FF1493"), // Deep Pink
            QColor("#00BFFF"), // Deep Sky Blue
            QColor("#FF4500"), // Orange Red
            QColor("#696969"), // Dim Gray
            QColor("#1E90FF"), // Dodger Blue
            QColor("#B22222"), // Firebrick
            QColor("#FF00FF"), // Fuchsia
            QColor("#FFD700"), // Gold
            QColor("#DAA520"), // Goldenrod
            QColor("#BEBEBE"), // Gray (X11)
            QColor("#7F7F7F"), // Gray (W3C)
            QColor("#007F00"), // Green (W3C)
            QColor("#ADFF2F"), // Green Yellow
            QColor("#CD5C5C"), // Indian Red
            QColor("#F0E68C"), // Khaki
            QColor("#7CFC00"), // Lawn Green
            QColor("#32CD32"), // Lime Green
            QColor("#FAF0E6"), // Linen
            QColor("#B03060"), // Maroon (X11)
            QColor("#0000CD"), // Medium Blue
            QColor("#66CDAA"), // Medium Aquamarine
            QColor("#BA55D3"), // Medium Orchid
            QColor("#3CB371"), // Medium Sea Green
            QColor("#7B68EE"), // Medium Slate Blue
            QColor("#00FA9A"), // Medium Spring Green
            QColor("#48D1CC"), // Medium Turquoise
            QColor("#C71585"), // Medium Violet Red
            QColor("#191970"), // Midnight Blue
            QColor("#000080"), // Navy
            QColor("#FFA500"), // Orange
            QColor("#7F0000"), // Maroon (W3C)
            QColor("#DA70D6"), // Orchid
            QColor("#DB7093"), // Pale Violet Red
            QColor("#9370DB"), // Medium Purple
            QColor("#CD853F"), // Peru
            QColor("#B0E0E6"), // Powder Blue
            QColor("#A020F0"), // Purple (X11)
            QColor("#7F007F"), // Purple (W3C)
            QColor("#BC8F8F"), // Rosy Brown
            QColor("#6B8E23"), // Olive Drab
            QColor("#4169E1"), // Royal Blue
            QColor("#8B4513"), // Saddle Brown
            QColor("#FA8072"), // Salmon
            QColor("#F4A460"), // Sandy Brown
            QColor("#2E8B57"), // Sea Green
            QColor("#A0522D"), // Sienna
            QColor("#C0C0C0"), // Silver
            QColor("#87CEEB"), // Sky Blue
            QColor("#6A5ACD"), // Slate Blue
            QColor("#708090"), // Slate Gray
            QColor("#00FF7F"), // Spring Green
            QColor("#4682B4"), // Steel Blue
            QColor("#D2B48C"), // Tan
            QColor("#008080"), // Teal
            QColor("#D8BFD8"), // Thistle
            QColor("#FF6347"), // Tomato
            QColor("#40E0D0"), // Turquoise
            QColor("#EE82EE"), // Violet
            QColor("#F5DEB3"), // Wheat
            QColor("#FFFF00"), // Yellow
            QColor("#9ACD32"), // Yellow Green
            QColor("#ADD8E6"), // Light Blue
            QColor("#F08080"), // Light Coral
            QColor("#E0FFFF"), // Light Cyan
            QColor("#FAFAD2"), // Light Goldenrod
            QColor("#D3D3D3"), // Light Gray
            QColor("#90EE90"), // Light Green
            QColor("#FFB6C1"), // Light Pink
            QColor("#FFA07A"), // Light Salmon
            QColor("#FFE4B5"), // Moccasin
            QColor("#20B2AA"), // Light Sea Green
            QColor("#87CEFA"), // Light Sky Blue
            QColor("#778899"), // Light Slate Gray
            QColor("#B0C4DE"), // Light Steel Blue
            QColor("#FFFFE0"), // Light Yellow
            QColor("#FFDAB9"), // Peach Puff
            QColor("#FFC0CB"), // Pink
            QColor("#DDA0DD"), // Plum
            QColor("#00FFFF"), // Aqua
            QColor("#7FFFD4"), // Aquamarine
            QColor("#FAEBD7"), // Antique White
            QColor("#F5F5DC"), // Beige
            QColor("#FFE4C4"), // Bisque
            QColor("#FFEBCD"), // Blanched Almond
            QColor("#00FFFF"), // Cyan
            QColor("#DEB887"), // Burlywood
            QColor("#FFF8DC"), // Cornsilk
            QColor("#FFEFD5"), // Papaya Whip
            QColor("#DCDCDC"), // Gainsboro
            QColor("#F0FFF0"), // Honeydew
            QColor("#FFFACD"), // Lemon Chiffon
            QColor("#FFE4E1"), // Misty Rose
            QColor("#FFDEAD"), // Navajo White
            QColor("#EEE8AA"), // Pale Goldenrod
            QColor("#98FB98"), // Pale Green
            QColor("#AFEEEE"), // Pale Turquoise

                    //          QColor("#F0F8FF"), // Alice Blue
                    //          QColor("#F0FFFF"), // Azure
                    //          QColor("#FFFAF0"), // Floral White
                    //          QColor("#F8F8FF"), // Ghost White
                    //          QColor("#FFFFF0"), // Ivory
                    //          QColor("#E6E6FA"), // Lavender
                    //          QColor("#FFF0F5"), // Lavender Blush
                    //          QColor("#F5FFFA"), // Mint Cream
                    //          QColor("#FDF5E6"), // Old Lace
                    //          QColor("#FFF5EE"), // Seashell
                    //          QColor("#FFFAFA"), // Snow
                    //          QColor("#FFFFFF"), // White
                    //          QColor("#F5F5F5"), // White Smoke
};

const QVector<QColor>& defaultColorSet()
{
    return __x11_colors;
}

} // namespace QCPL
