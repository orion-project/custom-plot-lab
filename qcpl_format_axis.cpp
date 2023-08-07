#include "qcpl_format_axis.h"

#include "qcpl_format.h"
#include "qcpl_format_editors.h"
#include "qcpl_io_json.h"
#include "qcpl_text_editor.h"
#include "qcustomplot/qcustomplot.h"

#include "helpers/OriLayouts.h"
#include "widgets/OriOptionsGroup.h"

#include <QCheckBox>
#include <QDebug>
#include <QFormLayout>
#include <QLabel>
#include <QLocale>
#include <QSpinBox>

using namespace Ori::Layouts;

static int __tabIndex = 0;

namespace QCPL {

AxisFormatWidget::AxisFormatWidget(QCPAxis* axis, const AxisFormatDlgProps& props) : QWidget(), _axis(axis)
{
    _backup = writeAxis(axis);
    _backup["text"] = axis->label();

    auto p = sizePolicy();
    p.setVerticalStretch(255);
    setSizePolicy(p);

    auto hintColor = palette().mid().color().name(QColor::HexRgb);

    //-------------------------------------------------------
    //                    Tab "Axis"

    TextEditorWidget::Options titleOpts;
    if (props.formatter)
    {
        _formatter = props.formatter;
        titleOpts.defaultText = props.defaultText;
        titleOpts.vars = _formatter->vars();
        _backup["formatter_text"] = _formatter->text();
    }
    _titleEditor = new TextEditorWidget(titleOpts);

    _innerMargin = makeSpinBox(-1000, 1000);
    _outerMargin = makeSpinBox(0, 1000);
    _offset = makeSpinBox(-2000, 2000);

    auto offsets = new QFormLayout;
    offsets->addRow(makeParamLabel(
        tr("Inner margin"), tr("distance between text and value labels"), hintColor), _innerMargin);
    offsets->addRow(makeParamLabel(
        tr("Outer margin"), tr("distance between text and diagram edge"), hintColor), _outerMargin);
    offsets->addRow(makeParamLabel(
        tr("Axis offset"), tr("displacement of axis line into plotting area"), hintColor), _offset);

    _logarithmic = new QCheckBox(tr("Logarithmic"));
    _reversed = new QCheckBox(tr("Reversed"));

    auto layoutPages = new QStackedLayout;

    layoutPages->addWidget(LayoutV({
        LayoutV({ _titleEditor }).makeGroupBox(tr("Title")),
        LayoutH({
            LayoutV({ offsets }).makeGroupBox(tr("Offsets")),
            LayoutV({ _logarithmic, _reversed }).makeGroupBox(tr("Scale")),
        })
    }).makeWidget());

    //-------------------------------------------------------
    //                   Tab "Labels"

    _labelsVisible = new QCheckBox(tr("Show labels"));
    _labelsInside = new QCheckBox(tr("Inside plotting area"));

    _labelsAngle = makeSpinBox(-90, 90);
    _labelsPadding = makeSpinBox(0, 100);

    auto labelsParams = new QFormLayout;
    labelsParams->addRow(makeParamLabel(
        tr("Angle"), tr("clockwise from -90° to 90°"), hintColor), _labelsAngle);
    labelsParams->addRow(makeParamLabel(
        tr("Margin"), tr("distance between text and axis line"), hintColor), _labelsPadding);

    TextEditorWidget::Options labelsOpts;
    labelsOpts.readOnly = true;
    labelsOpts.singleLine = true;
    _labelsEditor = new TextEditorWidget(labelsOpts);
    _labelsEditor->setText(
                QLocale::system().toString(3.141592653589793, 'f', 10) + "   " +
                QLocale::system().toString(147098290.0, 'e', 10) + "   " +
                QLocale::system().toString(365.256363004, 'g', 10)
                );
    _labelsEditor->singleLineEditor()->setAlignment(Qt::AlignCenter);
    _labelsEditor->singleLineEditor()->setTextMargins(6, 6, 6, 6);

    QString sampleNum = QLocale::system().toString(1.5);
    _numberFormat = new Ori::Widgets::OptionsGroup(tr("Number format"), true);
    _numberFormat->hintFormat = "<span style='color:" + hintColor + "'>(%1)</span>";
    _numberFormat->addOption(lnfF, tr("Fixed"), "1500000");
    _numberFormat->addOption(lnfE, tr("Exponential"), sampleNum+"e+06");
    _numberFormat->addOption(lnfG, tr("Automatic"), tr("take shortest"));

    _expFormat = new Ori::Widgets::OptionsGroup(tr("Exponent format"), true);
    _expFormat->hintFormat = _numberFormat->hintFormat;
    _expFormat->addOption(lefLowerE, tr("Lowercase e"), sampleNum+"e+06");
    _expFormat->addOption(lefUpperE, tr("Uppercase E"), sampleNum+"E+06");
    _expFormat->addOption(lefDot, tr("Dot mutiplication"), sampleNum+"·10<sup>6</sup>");
    _expFormat->addOption(lefCross, tr("Cross mutiplication"), sampleNum+"×10<sup>6</sup>");

    _labelsPrecision = makeSpinBox(1, 15);
    auto precisionHint = new QLabel(QString("<span style='color:%1'>%2</span>").arg(hintColor,
        tr("For fixed and exponential formats - number of digits after decimal point. "
            "For automatic format - maximum number of significant digits.")));
    precisionHint->setWordWrap(true);

    layoutPages->addWidget(LayoutV({
        LayoutH({
            LayoutV({
                _labelsVisible,
                _labelsInside
            }),
            Stretch(),
            labelsParams,
        }),
        SpaceV(2),
        LayoutH({
            _numberFormat,
            _expFormat,
            LayoutV({_labelsPrecision, precisionHint}).makeGroupBox(tr("Number precision")),
            Stretch(),
        }),
        SpaceV(2),
        _labelsEditor,
        Stretch(),
    }).makeWidget());

    //-------------------------------------------------------
    //                   Tab "Lines"

    PenEditorWidgetOptions penOpts;
    penOpts.noLabels = true;
    _axisPen = new PenEditorWidget(penOpts);
    _tickPen = new PenEditorWidget(penOpts);
    _tickLengthIn = makeSpinBox(0, 100);
    _tickLengthOut = makeSpinBox(0, 100);
    _subTickPen = new PenEditorWidget(penOpts);
    _subTickLengthIn = makeSpinBox(0, 100);
    _subTickLengthOut = makeSpinBox(0, 100);
    _gridPen = new PenEditorWidget(penOpts);
    _zeroPen = new PenEditorWidget(penOpts);
    _subGridPen = new PenEditorWidget(penOpts);
    _axisGroup = LayoutV({_axisPen}).makeGroupBox(tr("Axis"));
    _gridGroup = LayoutV({_gridPen}).makeGroupBox(tr("Primary Grid"));
    _gridGroup->setCheckable(true);
    _groupSubGrid = LayoutV({_subGridPen}).makeGroupBox(tr("Additional Grid"));
    _groupSubGrid->setCheckable(true);

    auto tickLen = new QFormLayout;
    tickLen->addRow(new QLabel(tr("Inner length:")), _tickLengthIn);
    tickLen->addRow(new QLabel(tr("Outer length:")), _tickLengthOut);
    _groupTicks = LayoutV({
        _tickPen,
        SpaceV(),
        LayoutH({tickLen, Stretch()}),
    }).makeGroupBox(tr("Primary ticks"));
    _groupTicks->setCheckable(true);

    auto subTickLen = new QFormLayout;
    subTickLen->addRow(new QLabel(tr("Inner length:")), _subTickLengthIn);
    subTickLen->addRow(new QLabel(tr("Outer length:")), _subTickLengthOut);
    _groupSubTicks = LayoutV({
        _subTickPen,
        SpaceV(),
        LayoutH({subTickLen, Stretch()}),
    }).makeGroupBox(tr("Additional ticks"));
    _groupSubTicks->setCheckable(true);

    layoutPages->addWidget(LayoutH({
        LayoutV({
            _axisGroup,
            _gridGroup,
            _groupTicks,
        }).setDefSpacing(2),
        LayoutV({
            LayoutV({_zeroPen}).makeGroupBox(tr("Zero Line")),
            _groupSubGrid,
            _groupSubTicks,
        }).setDefSpacing(2),
    }).setDefSpacing(2).makeWidget());

    _tabs = new QTabBar;
    _tabs->addTab("Axis");
    _tabs->addTab("Labels");
    _tabs->addTab("Lines");
    _tabs->setShape(QTabBar::TriangularNorth);
    connect(_tabs, &QTabBar::currentChanged, layoutPages, &QStackedLayout::setCurrentIndex);

    _visible = new QCheckBox("Visible");
    _saveDefault = new QCheckBox("Save as default");
    _saveDefault->setVisible(bool(props.onSaveDefault));

    auto header = makeDialogHeader();
    LayoutH({
        SpaceH(),
        LayoutV({ Stretch(), _visible, _saveDefault }).setMargin(6),
        LayoutV({ Stretch(), _tabs }).setMargin(0),
        SpaceH(2),
    }).setMargin(0).useFor(header);

    LayoutV({
        header,
        makeSeparator(),
        layoutPages,
    }).setSpacing(0).setMargin(0).useFor(this);

    //-------------------------------------------------------

    _visible->setChecked(axis->visible());
    if (props.formatter)
        _titleEditor->setText(props.formatter->text());
    else
        _titleEditor->setText(axis->label());
    _titleEditor->setFont(axis->labelFont());
    _titleEditor->setColor(axis->labelColor());
    _innerMargin->setValue(axis->labelPadding());
    _outerMargin->setValue(axis->padding());
    _offset->setValue(-axis->offset());
    _logarithmic->setChecked(axis->scaleType() == QCPAxis::stLogarithmic);
    _reversed->setChecked(axis->rangeReversed());
    _labelsVisible->setChecked(axis->tickLabels());
    _labelsInside->setChecked(axis->tickLabelSide() == QCPAxis::lsInside);
    _labelsAngle->setValue(axis->tickLabelRotation());
    _labelsPadding->setValue(axis->tickLabelPadding());
    _labelsEditor->setColor(axis->tickLabelColor());
    _labelsEditor->setFont(axis->tickLabelFont());

    _numberFormat->setOption(lnfG); // default value
    _expFormat->setOption(lefDot); // default value
    QString fmt = axis->numberFormat();
    if (fmt.size() > 0) {
        if (fmt.at(0) == 'f') {
            _numberFormat->setOption(lnfF);
        } else if (fmt.at(0) == 'e') {
            _numberFormat->setOption(lnfE);
            _expFormat->setOption(lefLowerE);
        } else if (fmt.at(0) == 'E') {
            _numberFormat->setOption(lnfE);
            _expFormat->setOption(lefUpperE);
        } else if (fmt.at(0) == 'g') {
            _numberFormat->setOption(lnfG);
            _expFormat->setOption(lefLowerE);
        } else if (fmt.at(0) == 'G') {
            _numberFormat->setOption(lnfG);
            _expFormat->setOption(lefUpperE);
        }
        if (fmt.size() > 1 && fmt.at(1) == 'b') {
            if (fmt.size() > 2 && fmt.at(2) == 'c') {
                _expFormat->setOption(lefCross);
            } else {
                _expFormat->setOption(lefDot);
            }
        }
    }

    _labelsPrecision->setValue(axis->numberPrecision());
    _axisPen->setValue(axis->basePen());
    _groupTicks->setChecked(axis->ticks());
    _tickPen->setValue(axis->tickPen());
    _tickLengthIn->setValue(axis->tickLengthIn());
    _tickLengthOut->setValue(axis->tickLengthOut());
    _groupTicks->setChecked(axis->subTicks());
    _subTickPen->setValue(axis->subTickPen());
    _subTickLengthIn->setValue(axis->subTickLengthIn());
    _subTickLengthOut->setValue(axis->subTickLengthOut());
    auto grid = axis->grid();
    _gridGroup->setChecked(grid->visible());
    _gridPen->setValue(grid->pen());
    _zeroPen->setValue(grid->zeroLinePen());
    _groupSubGrid->setChecked(grid->subGridVisible());
    _subGridPen->setValue(grid->subGridPen());

    _tabs->setCurrentIndex(__tabIndex);
    layoutPages->setCurrentIndex(__tabIndex);
}

AxisFormatWidget::~AxisFormatWidget()
{
    __tabIndex = _tabs->currentIndex();
}

void AxisFormatWidget::restore()
{
    readAxis(_backup, _axis);
    _axis->setLabel(_backup["text"].toString());
    if (_formatter)
        _formatter->setText(_backup["formatter_text"].toString());
    _axis->parentPlot()->replot();
}

void AxisFormatWidget::apply()
{
    _axis->setVisible(_visible->isChecked());
    if (_formatter)
    {
        _formatter->setText(_titleEditor->text());
        _formatter->format();
    }
    else _axis->setLabel(_titleEditor->text());
    _axis->setLabelFont(_titleEditor->font());
    _axis->setSelectedLabelFont(_titleEditor->font());
    _axis->setLabelColor(_titleEditor->color());
    _axis->setLabelPadding(_innerMargin->value());
    _axis->setPadding(_outerMargin->value());
    _axis->setOffset(-_offset->value());
    _axis->setScaleType(_logarithmic->isChecked() ? QCPAxis::stLogarithmic : QCPAxis::stLinear);
    _axis->setRangeReversed(_reversed->isChecked());
    _axis->setTickLabels(_labelsVisible->isChecked());
    _axis->setTickLabelColor(_labelsEditor->color());
    _axis->setTickLabelFont(_labelsEditor->font());
    _axis->setSelectedTickLabelFont(_labelsEditor->font());
    _axis->setTickLabelRotation(_labelsAngle->value());
    _axis->setTickLabelPadding(_labelsPadding->value());
    _axis->setTickLabelSide(_labelsInside->isChecked() ? QCPAxis::lsInside : QCPAxis::lsOutside);
    _axis->setNumberPrecision(_labelsPrecision->value());

    char fmt[3] = {'\0', '\0', '\0'};
    if (_numberFormat->option() == lnfF) {
        fmt[0] = 'f';
    } else if (_numberFormat->option() == lnfE) {
        fmt[0] = _expFormat->option() == lefUpperE ? 'E' : 'e';
    } else {
        fmt[0] = _expFormat->option() == lefUpperE ? 'G' : 'g';
    }
    if (_expFormat->option() == lefDot) {
        fmt[1] = 'b';
    } else if (_expFormat->option() == lefCross) {
        fmt[1] = 'b';
        fmt[2] = 'c';
    }
    _axis->setNumberFormat(QString::fromLatin1(fmt));
    _axis->setBasePen(_axisPen->value());
    _axis->setTicks(_groupTicks->isChecked());
    _axis->setTickPen(_tickPen->value());
    _axis->setTickLengthIn(_tickLengthIn->value());
    _axis->setTickLengthOut(_tickLengthOut->value());
    _axis->setSubTicks(_groupTicks->isChecked());
    _axis->setSubTickPen(_subTickPen->value());
    _axis->setSubTickLengthIn(_subTickLengthIn->value());
    _axis->setSubTickLengthOut(_subTickLengthOut->value());
    auto grid = _axis->grid();
    grid->setVisible(_gridGroup->isChecked());
    grid->setPen(_gridPen->value());
    grid->setZeroLinePen(_zeroPen->value());
    grid->setSubGridVisible(_groupSubGrid->isChecked());
    grid->setSubGridPen(_subGridPen->value());

    _axis->parentPlot()->replot();
}

bool AxisFormatWidget::needSaveDefault() const
{
    return _saveDefault->isChecked();
}

} // namespace QCPL
