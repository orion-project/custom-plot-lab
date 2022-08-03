#include "qcpl_format_legend.h"

#include "qcpl_format_editors.h"
#include "qcpl_plot.h"
#include "qcpl_text_editor.h"

#include "helpers/OriDialogs.h"
#include "helpers/OriLayouts.h"
#include "widgets/OriSelectableTile.h"

#include <QAction>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>

using namespace Ori::Layouts;

namespace QCPL {

LegendFormatWidget::LegendFormatWidget(QCPLegend *legend) : QWidget(), _legend(legend)
{
    TextEditorWidget::Options opts;
    opts.showBackColor = true;
    opts.colorAlphaBack = true;
    _textProps = new TextEditorWidget(opts);
    _textProps->setText("spectrum1.txt\nprofile3.dat [1;2]\nClipboard 3 [3;4]");
    _textProps->setFont(legend->font());
    _textProps->setColor(legend->textColor());
    _textProps->setBackColor(legend->brush().color());

    auto actnBorderFormat = new QAction(QIcon(":/qcpl_images/frame"), tr("Border format"), this);
    connect(actnBorderFormat, &QAction::triggered, this, &LegendFormatWidget::borderFormat);
    _textProps->addAction(actnBorderFormat);
    _borderPen = legend->borderPen();

    _iconW = new QSpinBox;
    _iconW->setValue(_legend->iconSize().width());

    _iconH = new QSpinBox;
    _iconH->setValue(_legend->iconSize().height());

    _iconMargin = new QSpinBox;
    _iconMargin->setValue(_legend->iconTextPadding());

    auto iconGroup = new QGroupBox(tr("Icon"));
    auto iconLayout = new QFormLayout(iconGroup);
    iconLayout->addRow(tr("Width"), _iconW);
    iconLayout->addRow(tr("Height"), _iconH);
    iconLayout->addRow(tr("Margin"), _iconMargin);

    _paddings = new MarginsEditorWidget(tr("Paddings"));
    _paddings->setValue(legend->margins());

    _margins = new MarginsEditorWidget(tr("Margins"));
    _margins->setValue(_legend->parentPlot()->axisRect()->insetLayout()->margins());

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
    _locationGroup->selectData(int(_legend->parentPlot()->axisRect()->insetLayout()->insetAlignment(0)));

    LayoutV({
                _textProps,
                LayoutH({iconGroup, _paddings, locationGroup, _margins}),
                Stretch(),
            }).setMargin(0).useFor(this);
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
    _legend->setBrush(_textProps->backColor());
    _legend->setFont(_textProps->font());
    _legend->setSelectedFont(_textProps->font());
    _legend->setTextColor(_textProps->color());
    _legend->setIconSize({_iconW->value(), _iconH->value()});
    _legend->setIconTextPadding(_iconMargin->value());
    _legend->setMargins(_paddings->value());
    // https://www.qcustomplot.com/index.php/tutorials/basicplotting
    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
    _legend->parentPlot()->axisRect()->insetLayout()->setInsetAlignment(0, Qt::Alignment(_locationGroup->selectedData().toInt()));
    _legend->parentPlot()->axisRect()->insetLayout()->setMargins(_margins->value());
    _legend->setBorderPen(_borderPen);
}

void LegendFormatWidget::borderFormat()
{
    // TODO: use inline editor, no need for dialog
    PenEditorWidget editor;
    editor.setValue(_borderPen);

    bool ok = Ori::Dlg::Dialog(&editor, false)
            .withTitle(tr("Border Format"))
            .exec();
    if (ok)
        _borderPen = editor.value();
}

} // namespace QCPL
