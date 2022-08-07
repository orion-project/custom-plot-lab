#include "qcpl_format_axis.h"

#include "qcpl_format_editors.h"
#include "qcpl_plot.h"
#include "qcpl_text_editor.h"

#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"
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

AxisFormatWidget::AxisFormatWidget(QCPAxis* axis) : QTabWidget(), _axis(axis)
{
    auto p = sizePolicy();
    p.setVerticalStretch(255);
    setSizePolicy(p);

    //-------------------------------------------------------

    TextEditorWidget::Options titleOpts;
    _titleEditor = new TextEditorWidget(titleOpts);
    _titleEditor->setText(axis->label());
    _titleEditor->setFont(axis->labelFont());
    _titleEditor->setColor(axis->labelColor());

    _innerMargin = new QSpinBox;
    _innerMargin->setValue(axis->labelPadding());

    _outerMargin = new QSpinBox;
    _outerMargin->setValue(axis->padding());

    auto titleParams = new QFormLayout;
    titleParams->addRow(makeParamLabel(tr("Inner margin"), tr("distance between text and value labels")), _innerMargin);
    titleParams->addRow(makeParamLabel(tr("Outer margin"), tr("distance between text and diagram edge")), _outerMargin);

    // TODO: move to the second page when it's ready
    _logarithmic = new QCheckBox(tr("Logarithmic"));
    _logarithmic->setChecked(axis->scaleType() == QCPAxis::stLogarithmic);

    _reversed = new QCheckBox(tr("Reversed"));
    _reversed->setChecked(axis->rangeReversed());

    addTab(LayoutV({
                       makeLabelSeparator(tr("Title")),
                       _titleEditor,
                       Space(10),
                       LayoutH({titleParams, Stretch()}),
                       Space(10),
                       makeLabelSeparator(tr("Scale")),
                       _logarithmic,
                       _reversed,
                   }).makeWidget(), tr("Axis"));

    //-------------------------------------------------------

// TODO
//    _offset = new QSpinBox;
//    _offset->setValue(axis->offset());

//    addTab(LayoutV({
//                       _logarithmic,
//                       _reversed,
//                       LayoutH({paramLabel(tr("Offset"), tr("distance between line and plotting area")), _offset}),
//                       Stretch(),
//                   }).makeWidget(), tr("Axis"));

    //-------------------------------------------------------

    TextEditorWidget::Options labelsOpts;
    _labelsEditor = new TextEditorWidget(labelsOpts);
    _labelsEditor->setText(
                QLocale::system().toString(3.141592653589793, 'f', 10) + "   " +
                QLocale::system().toString(147098290.0, 'e', 10) + "   " +
                QLocale::system().toString(365.256363004, 'g', 10)
                );
    _labelsEditor->setColor(axis->tickLabelColor());
    _labelsEditor->setFont(axis->tickLabelFont());

    _labelsAngle = new QSpinBox;
    _labelsAngle->setRange(-90, 90);
    _labelsAngle->setValue(axis->tickLabelRotation());

    _labelsPadding = new QSpinBox;
    _labelsPadding->setValue(axis->tickLabelPadding());

    _labelsVisible = new QCheckBox(tr("Show labels"));
    _labelsVisible->setChecked(axis->tickLabels());

    _labelsInside = new QCheckBox(tr("Inside plotting area"));
    _labelsInside->setChecked(axis->tickLabelSide() == QCPAxis::lsInside);

    auto labelsParams = new QFormLayout;
    labelsParams->addRow(makeParamLabel(tr("Angle"), tr("clockwise from -90° to 90°")), _labelsAngle);
    labelsParams->addRow(makeParamLabel(tr("Margin"), tr("distance between text and axis line")), _labelsPadding);

    QString sampleNum = QLocale::system().toString(1.5);
    _numberFormat = new Ori::Widgets::OptionsGroup(tr("Number format"), true);
    // TODO: take some color from palette, don't use hardcoded 'gray'
    _numberFormat->hintFormat = "<span style='color:gray'>(%1)</span>";
    _numberFormat->addOption(lnfF, tr("Fixed"), "1500000");
    _numberFormat->addOption(lnfE, tr("Exponential"), sampleNum+"e+06");
    _numberFormat->addOption(lnfG, tr("Automatic"), tr("take shortest"));

    _expFormat = new Ori::Widgets::OptionsGroup(tr("Exponent format"), true);
    // TODO: take some color from palette, don't use hardcoded 'gray'
    _expFormat->hintFormat = "<span style='color:gray'>(%1)</span>";
    _expFormat->addOption(lefLowerE, tr("Lowercase e"), sampleNum+"e+06");
    _expFormat->addOption(lefUpperE, tr("Uppercase E"), sampleNum+"E+06");
    _expFormat->addOption(lefDot, tr("Dot mutiplication"), sampleNum+"·10<sup>6</sup>");
    _expFormat->addOption(lefCross, tr("Cross mutiplication"), sampleNum+"×10<sup>6</sup>");

    // default values
    _numberFormat->setOption(lnfG);
    _expFormat->setOption(lefDot);

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

    _labelsPrecision = new QSpinBox;
    _labelsPrecision->setValue(axis->numberPrecision());
    auto precisionHint = new QLabel(tr("<span style='color:gray'>"
       "For fixed and exponential formats, a number of digits after decimal point. "
       "For automatic format, maximum number of significant digits.</span>"));
    precisionHint->setWordWrap(true);

    addTab(LayoutV({
                       LayoutH({LayoutV({
                                    _labelsVisible,
                                    _labelsInside
                                }),
                                Stretch(),
                                labelsParams,
                       }),
                       Space(10),
                       LayoutH({
                           _numberFormat,
                           _expFormat,
                           Ori::Gui::groupV(tr("Number precision"), {_labelsPrecision, precisionHint}),
                           Stretch(),
                       }),
                       Space(10),
                       _labelsEditor,
                       Stretch(),
                   }).makeWidget(), tr("Labels"));

    //-------------------------------------------------------

    //addTab(LayoutV({}).makeWidget(), tr("Grid"));

    //-------------------------------------------------------

    setCurrentIndex(__tabIndex);
}

void AxisFormatWidget::apply()
{
    _axis->setLabel(_titleEditor->text());
    _axis->setLabelFont(_titleEditor->font());
    _axis->setSelectedLabelFont(_titleEditor->font());
    _axis->setLabelColor(_titleEditor->color());
    _axis->setLabelPadding(_innerMargin->value());
    _axis->setPadding(_outerMargin->value());
   // _axis->setOffset(_offset->value());
    _axis->setScaleType(_logarithmic->isChecked() ? QCPAxis::stLogarithmic : QCPAxis::stLinear);
    _axis->setRangeReversed(_reversed->isChecked());
    _axis->setTickLabels(_labelsVisible->isChecked());
    // TODO: take some contrast color if the blue choosen as the base
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

    __tabIndex = currentIndex();
}

} // namespace QCPL
