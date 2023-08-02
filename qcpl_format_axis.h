#ifndef QCPL_FORMAT_AXIS_H
#define QCPL_FORMAT_AXIS_H

#include <QTabWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGroupBox;
class QSpinBox;
QT_END_NAMESPACE

namespace Ori {
namespace Widgets {
class OptionsGroup;
}}

class QCPAxis;

namespace QCPL {

class PenEditorWidget;
class TextEditorWidget;

class AxisFormatWidget : public QTabWidget
{
    Q_OBJECT

public:
    AxisFormatWidget(QCPAxis* axis);

public slots:
    void apply();

private:
    enum LabelsNumberFormat { lnfF, lnfE, lnfG };
    enum LabelsExpFormat { lefLowerE, lefUpperE, lefDot, lefCross };

    QCPAxis *_axis;
    TextEditorWidget *_titleEditor, *_labelsEditor;
    QSpinBox *_outerMargin, *_innerMargin, *_offset, *_labelsAngle, *_labelsPadding,
        *_labelsPrecision, *_tickLengthIn, *_tickLengthOut, *_subTickLengthIn, *_subTickLengthOut;
    QCheckBox *_logarithmic, *_reversed, *_labelsVisible, *_labelsInside;
    QGroupBox *_axisGroup, *_gridGroup, *_groupSubGrid, *_groupTicks, *_groupSubTicks;
    Ori::Widgets::OptionsGroup *_numberFormat, *_expFormat;
    PenEditorWidget *_axisPen, *_tickPen, *_subTickPen, *_gridPen, *_zeroPen, *_subGridPen;
};

} // namespace QCPL

#endif // QCPL_FORMAT_AXIS_H
