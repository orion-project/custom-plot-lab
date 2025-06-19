#ifndef QCPL_FORMAT_AXIS_H
#define QCPL_FORMAT_AXIS_H

#include <QWidget>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGroupBox;
class QSpinBox;
class QTabBar;
QT_END_NAMESPACE

namespace Ori::Widgets {
class OptionsGroup;
class SelectableTileRadioGroup;
class ValueEdit;
}

class QCPAxis;
class QCPColorScale;

namespace QCPL {

struct AxisFormatDlgProps;
class MarginsEditorWidget;
class PenEditorWidget;
class TextEditorWidget;
class TextFormatterBase;

class AxisFormatWidget : public QWidget
{
    Q_OBJECT

public:
    struct Props
    {
        QString defaultText;
        TextFormatterBase *formatter = nullptr;
        bool hasSaveDefault = false;
        QCPColorScale *colorScale = nullptr;
    };

    AxisFormatWidget(QCPAxis* axis, const Props &props);
    ~AxisFormatWidget();

    bool needSaveDefault() const;
    void populate();

public slots:
    void apply();
    void restore();

private:
    enum LabelsNumberFormat { lnfF, lnfE, lnfG };
    enum LabelsExpFormat { lefLowerE, lefUpperE, lefDot, lefCross };

    QCPAxis *_axis;
    QCPColorScale *_scale;
    QJsonObject _backup;
    TextFormatterBase *_formatter = nullptr;
    QTabBar *_tabs;
    TextEditorWidget *_titleEditor, *_labelsEditor;
    QSpinBox *_outerMargin, *_innerMargin, *_offset, *_labelsAngle, *_labelsPadding,
        *_labelsPrecision, *_tickLengthIn, *_tickLengthOut, *_subTickLengthIn, *_subTickLengthOut,
        *_colorScaleWidth, *_tickCount;
    QCheckBox *_visible, *_saveDefault, *_logarithmic, *_reversed, *_labelsVisible, *_labelsInside,
        *_tickerReadability;
    QGroupBox *_axisGroup, *_gridGroup, *_groupSubGrid, *_groupTicks, *_groupSubTicks;
    Ori::Widgets::OptionsGroup *_numberFormat, *_expFormat;
    PenEditorWidget *_axisPen, *_tickPen, *_subTickPen, *_gridPen, *_zeroPen, *_subGridPen;
    MarginsEditorWidget *_colorScaleMargins;
    Ori::Widgets::SelectableTileRadioGroup *_gradientGroup;
    Ori::Widgets::ValueEdit *_tickOffset;
};

} // namespace QCPL

#endif // QCPL_FORMAT_AXIS_H
