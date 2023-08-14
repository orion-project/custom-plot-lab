#include "qcpl_format_axis.h"

#include "qcpl_format.h"
#include "qcpl_format_editors.h"
#include "qcpl_io_json.h"
#include "qcpl_text_editor.h"
#include "qcpl_utils.h"
#include "qcustomplot/qcustomplot.h"

#include "helpers/OriLayouts.h"
#include "widgets/OriOptionsGroup.h"
#include "widgets/OriSelectableTile.h"
#include "widgets/OriValueEdit.h"

#include <QCheckBox>
#include <QDebug>
#include <QFormLayout>
#include <QLabel>
#include <QLocale>
#include <QSpinBox>

using namespace Ori::Layouts;

static int __tabIndex = 0;

namespace QCPL {

//------------------------------------------------------------------------------
//                               AxisFormatWidget
//------------------------------------------------------------------------------

class SelectableTileContentGradient : public Ori::Widgets::SelectableTileContent
{
public:
    SelectableTileContentGradient(QCPAxis::AxisType axisType, QCPColorGradient::GradientPreset gradient)
        : _axisType(axisType), _gradient(QCPColorGradient(gradient))
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.drawImage(0, 0, _image);
    }

    void resizeEvent(QResizeEvent *event) override
    {
        if (_image.size() != event->size())
            updateGradientImage();
    }

private:
    QCPAxis::AxisType _axisType;
    QCPColorGradient _gradient;
    QImage _image;

    // The same code as in QCPColorScaleAxisRectPrivate::updateGradientImage()
    void updateGradientImage()
    {
        if (rect().isEmpty())
            return;
        const QImage::Format format = QImage::Format_ARGB32_Premultiplied;
        int n = _gradient.levelCount();
        int w, h;
        QVector<double> data(n);
        for (int i = 0; i < n; i++)
            data[i] = i;
        if (_axisType == QCPAxis::atBottom || _axisType == QCPAxis::atTop)
        {
            w = n;
            h = rect().height();
            _image = QImage(w, h, format);
            QVector<QRgb*> pixels;
            for (int y = 0; y < h; y++)
                pixels.append(reinterpret_cast<QRgb*>(_image.scanLine(y)));
            _gradient.colorize(data.constData(), QCPRange(0, n-1), pixels.first(), n);
            for (int y = 1; y < h; y++)
                memcpy(pixels.at(y), pixels.first(), size_t(n)*sizeof(QRgb));
        }
        else
        {
            w = rect().width();
            h = n;
            _image = QImage(w, h, format);
            for (int y = 0; y < h; y++)
            {
                QRgb *pixels = reinterpret_cast<QRgb*>(_image.scanLine(y));
                const QRgb lineColor = _gradient.color(data[h-1-y], QCPRange(0, n-1));
                for (int x = 0; x < w; x++)
                    pixels[x] = lineColor;
            }
        }
    }
};

//------------------------------------------------------------------------------
//                               AxisFormatWidget
//------------------------------------------------------------------------------

AxisFormatWidget::AxisFormatWidget(QCPAxis* axis, const Props &props) :
    QWidget(), _axis(axis), _scale(props.colorScale)
{
    if (_scale)
        _backup = writeColorScale(_scale);
    else
        _backup = writeAxis(_axis);
    _backup["text"] = axis->label();

    auto p = sizePolicy();
    p.setVerticalStretch(255);
    setSizePolicy(p);

    auto hintColor = palette().mid().color().name(QColor::HexRgb);

    //-------------------------------------------------------

    auto layoutPages = new QStackedLayout;

    _tabs = new QTabBar;
    _tabs->setShape(QTabBar::TriangularNorth);
    connect(_tabs, &QTabBar::currentChanged, layoutPages, &QStackedLayout::setCurrentIndex);

    _visible = new QCheckBox("Visible");
    _saveDefault = new QCheckBox("Save as default");

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

    _tickerReadability = new QCheckBox(tr("Prefer readability, not exact count"));
    _tickCount = makeSpinBox(1, 500);
    _tickOffset = new Ori::Widgets::ValueEdit();
    _tickOffset->setPreferredWidth(100);

    layoutPages->addWidget(LayoutV({
        LayoutV({ _titleEditor }).makeGroupBox(tr("Title")),
        LayoutH({
            LayoutV({ offsets }).makeGroupBox(tr("Offsets")),
            LayoutV({ _logarithmic, _reversed }).makeGroupBox(tr("Scale")),
        }),
        LayoutH({
            _tickerReadability,
            SpaceH(2),
            new QLabel(tr("Tick count:")),
            _tickCount,
            SpaceH(2),
            new QLabel(tr("Tick offset:")),
            _tickOffset,
            Stretch(),
        }).makeGroupBox(tr("Ticker"))
    }).makeWidget());
    _tabs->addTab("Axis");

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

    _labelsPrecision = makeSpinBox(0, 15);
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
        LayoutV({
            _labelsEditor,
        }).makeGroupBox(tr("Font")),
    }).makeWidget());
    _tabs->addTab("Labels");

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
        SpaceV(2),
        LayoutH({tickLen, Stretch()}),
    }).makeGroupBox(tr("Primary ticks"));
    _groupTicks->setCheckable(true);

    auto subTickLen = new QFormLayout;
    subTickLen->addRow(new QLabel(tr("Inner length:")), _subTickLengthIn);
    subTickLen->addRow(new QLabel(tr("Outer length:")), _subTickLengthOut);
    _groupSubTicks = LayoutV({
        _subTickPen,
        SpaceV(2),
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
    _tabs->addTab("Lines");

    //-------------------------------------------------------
    //                   Tab "Gradient"

    QVector<QCPColorGradient::GradientPreset> gradientTypes;

    if (_scale)
    {
        MarginsEditorWidget::Options marginOpts;
        marginOpts.layoutInLine = true;
        _colorScaleMargins = new MarginsEditorWidget(tr("Margins"), marginOpts);

        auto axisType = _axis->axisType();
        _gradientGroup = new Ori::Widgets::SelectableTileRadioGroup(this);
        QBoxLayout *gradientLayout;
        if (axisType == QCPAxis::atBottom || axisType == QCPAxis::atTop)
            gradientLayout = new QVBoxLayout;
        else gradientLayout = new QHBoxLayout;
        gradientTypes = {
            QCPColorGradient::gpGrayscale,
            QCPColorGradient::gpHot,
            QCPColorGradient::gpCold,
            QCPColorGradient::gpNight,
            QCPColorGradient::gpCandy,
            QCPColorGradient::gpGeography,
            QCPColorGradient::gpIon,
            QCPColorGradient::gpThermal,
            QCPColorGradient::gpPolar,
            QCPColorGradient::gpSpectrum,
            QCPColorGradient::gpJet,
            QCPColorGradient::gpHues,
        };
        for (auto preset : gradientTypes)
        {
            auto tile = new Ori::Widgets::SelectableTile(new SelectableTileContentGradient(axisType, preset));
            tile->setData(int(preset));
            _gradientGroup->addTile(tile);
            gradientLayout->addWidget(tile);
        }

        _colorScaleWidth = makeSpinBox(10, 100);

        // TODO: the layout is not adjusted for horizonatl gradients
        layoutPages->addWidget(LayoutV({
            LayoutH({
                _colorScaleMargins,
                LayoutH({ new QLabel("Bar width:"), _colorScaleWidth }).makeGroupBox(tr("Size")),
            }),
            SpaceV(3),
            gradientLayout,
        }).setDefSpacing(2).makeWidget());
        _tabs->addTab("Gradient");
    }

    //-------------------------------------------------------

    if (_scale)
        _visible->setChecked(_scale->visible());
    else
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
    if (_scale)
    {
        _colorScaleMargins->setValue(_scale->margins());
        // We don't support arbitrary gradients, only selection from presets
        auto gradient = _scale->gradient();
        for (auto preset : gradientTypes)
            if (gradient == QCPColorGradient(preset)) {
                _gradientGroup->selectData(int(preset));
                break;
            }
        _colorScaleWidth->setValue(_scale->barWidth());
    }
    auto ticker = axis->ticker();
    _tickerReadability->setChecked(ticker->tickStepStrategy() == QCPAxisTicker::tssReadability);
    _tickCount->setValue(ticker->tickCount());
    _tickOffset->setValue(ticker->tickOrigin());

    // toggle visibility after adding to layout,
    // otherwise widget can flash on the screen as top-level window
    _saveDefault->setVisible(props.hasSaveDefault);

    _tabs->setCurrentIndex(__tabIndex);
    layoutPages->setCurrentIndex(__tabIndex);
}

AxisFormatWidget::~AxisFormatWidget()
{
    __tabIndex = _tabs->currentIndex();
}

void AxisFormatWidget::restore()
{
    if (_scale)
        readColorScale(_backup, _scale);
    else
        readAxis(_backup, _axis);
    _axis->setLabel(_backup["text"].toString());
    if (_formatter)
        _formatter->setText(_backup["formatter_text"].toString());
    _axis->parentPlot()->replot();
}

void AxisFormatWidget::apply()
{
    if (_scale)
        _scale->setVisible(_visible->isChecked());
    else
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
    if (_scale)
    {
        _scale->setMargins(_colorScaleMargins->value());
        auto selectedGradient = _gradientGroup->selectedData();
        if (!selectedGradient.isNull() and selectedGradient.isValid())
            _scale->setGradient(QCPColorGradient::GradientPreset(selectedGradient.toInt()));
        _scale->setBarWidth(_colorScaleWidth->value());
    }
    auto ticker = _axis->ticker();
    ticker->setTickStepStrategy(_tickerReadability->isChecked() ? QCPAxisTicker::tssReadability : QCPAxisTicker::tssMeetTickCount);
    ticker->setTickCount(_tickCount->value());
    ticker->setTickOrigin(_tickOffset->value());
    updateAxisTicker(_axis);
    _axis->parentPlot()->replot();
}

bool AxisFormatWidget::needSaveDefault() const
{
    return _saveDefault->isChecked();
}

} // namespace QCPL
