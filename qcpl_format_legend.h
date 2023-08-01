#ifndef QCPL_FORMAT_LEGEND_H
#define QCPL_FORMAT_LEGEND_H

#include <QWidget>
#include <QPen>

class QCPLegend;

namespace Ori {
namespace Widgets {
class SelectableTileRadioGroup;
}}

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGridLayout;
class QSpinBox;
QT_END_NAMESPACE

namespace QCPL {

class PenEditorWidget;
class TextEditorWidget;
class MarginsEditorWidget;
struct LegendFormatDlgProps;

class LegendFormatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LegendFormatWidget(QCPLegend *legend, const LegendFormatDlgProps& props);

public slots:
    void apply();

private:
    QCPLegend *_legend;
    Ori::Widgets::SelectableTileRadioGroup *_locationGroup;
    QGridLayout *_locationLayout;
    TextEditorWidget *_textProps;
    QSpinBox *_iconW, *_iconH, *_iconMargin;
    MarginsEditorWidget *_margins, *_paddings;
    QCheckBox *_visible, *_saveDefault;
    PenEditorWidget *_borderPen;
    std::function<void()> onApplied;
    std::function<void()> onSaveDefault;

    void makeLocationTile(Qt::Alignment align, int row, int col);
    void borderFormat();
};

} // namespace QCPL

#endif // QCPL_FORMAT_LEGEND_H
