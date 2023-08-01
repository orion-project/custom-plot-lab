#include "qcpl_format_legend.h"

#include "qcpl_format.h"
#include "qcpl_format_editors.h"
#include "qcpl_text_editor.h"
#include "qcpl_utils.h"
#include "qcustomplot/qcustomplot.h"

#include "helpers/OriLayouts.h"
#include "widgets/OriLabels.h"
#include "widgets/OriSelectableTile.h"

#include <QAction>
#include <QDebug>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>

using namespace Ori::Layouts;

namespace QCPL {

LegendFormatWidget::LegendFormatWidget(QCPLegend *legend, const LegendFormatDlgProps& props) : QWidget(), _legend(legend)
{
    onApplied = props.onApplied;

    TextEditorWidget::Options textOpts;
    textOpts.showBackColor = true;
    textOpts.colorAlphaBack = true;
    textOpts.narrow = true;
    _textProps = new TextEditorWidget(textOpts);
    _textProps->setText(props.sampleText.isEmpty() ? QString("spectrum1.txt\nprofile3.dat [1;2]\nClipboard 3 [3;4]") : props.sampleText);
    auto textGroup = new QGroupBox(tr("Font"));
    LayoutV({_textProps}).useFor(textGroup);

    PenEditorWidgetOptions penOpts;
    penOpts.noLabels = true;
    _borderPen = new PenEditorWidget(penOpts);
    auto borderGroup = new QGroupBox(tr("Border"));
    LayoutV({_borderPen}).useFor(borderGroup);

    _iconW = new QSpinBox;
    _iconH = new QSpinBox;
    _iconMargin = new QSpinBox;

    auto iconGroup = new QGroupBox(tr("Icon"));
    auto iconLayout = new QFormLayout(iconGroup);
    iconLayout->addRow(tr("Width"), _iconW);
    iconLayout->addRow(tr("Height"), _iconH);
    iconLayout->addRow(tr("Margin"), _iconMargin);

    _paddings = new MarginsEditorWidget(tr("Paddings"));
    _margins = new MarginsEditorWidget(tr("Margins"));

    _locationGroup = new Ori::Widgets::SelectableTileRadioGroup(this);
    auto locationGroup = new QGroupBox(tr("Location"));
    _locationLayout = new QGridLayout(locationGroup);
    _locationLayout->setSpacing(0);
    makeLocationTile(Qt::AlignLeft|Qt::AlignTop, 0, 0);
    makeLocationTile(Qt::AlignHCenter|Qt::AlignTop, 0, 1);
    makeLocationTile(Qt::AlignRight|Qt::AlignTop, 0, 2);
    makeLocationTile(Qt::AlignLeft|Qt::AlignVCenter, 1, 0);
    makeLocationTile(Qt::AlignHCenter|Qt::AlignVCenter, 1, 1);
    makeLocationTile(Qt::AlignRight|Qt::AlignVCenter, 1, 2);
    makeLocationTile(Qt::AlignLeft|Qt::AlignBottom, 2, 0);
    makeLocationTile(Qt::AlignHCenter|Qt::AlignBottom, 2, 1);
    makeLocationTile(Qt::AlignRight|Qt::AlignBottom, 2, 2);

    _visible = new QCheckBox(tr("Visible"));
    _visible->setChecked(legend->visible());

    auto separator = new Ori::Widgets::LabelSeparator;
    separator->flat = true;

    LayoutV({
                _visible,
                separator,
                textGroup,
                borderGroup,
                LayoutV({
                    LayoutH({iconGroup, _paddings}),
                    LayoutH({locationGroup, _margins}),
                }),
                Stretch(),
            }).setMargin(0).useFor(this);

    _textProps->setFont(legend->font());
    _textProps->setColor(legend->textColor());
    _textProps->setBackColor(legend->brush().color());
    _borderPen->setValue(legend->borderPen());
    _iconW->setValue(_legend->iconSize().width());
    _iconH->setValue(_legend->iconSize().height());
    _iconMargin->setValue(_legend->iconTextPadding());
    _paddings->setValue(legend->margins());
    _margins->setValue(legendLayout(_legend)->margins());
    _locationGroup->selectData(int(legendLayout(_legend)->insetAlignment(0)));
}

void LegendFormatWidget::makeLocationTile(Qt::Alignment align, int row, int col)
{
    auto tile = new Ori::Widgets::SelectableTile;
    tile->setData(int(align));
    tile->setFixedSize(24, 24);
    _locationGroup->addTile(tile);
    _locationLayout->addWidget(tile, row, col);
}

void LegendFormatWidget::apply()
{
    _legend->setFont(_textProps->font());
    _legend->setSelectedFont(_textProps->font());
    _legend->setBrush(_textProps->backColor());
    _legend->setTextColor(_textProps->color());
    _legend->setBorderPen(_borderPen->value());
    _legend->setIconSize({_iconW->value(), _iconH->value()});
    _legend->setIconTextPadding(_iconMargin->value());
    _legend->setMargins(_paddings->value());
    // https://www.qcustomplot.com/index.php/tutorials/basicplotting
    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
    legendLayout(_legend)->setInsetAlignment(0, Qt::Alignment(_locationGroup->selectedData().toInt()));
    legendLayout(_legend)->setMargins(_margins->value());
    _legend->setVisible(_visible->isChecked());
    if (onApplied) onApplied();
}

} // namespace QCPL
