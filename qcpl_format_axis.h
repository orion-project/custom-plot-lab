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

namespace Ori {
namespace Widgets {
class OptionsGroup;
}}

class QCPAxis;

namespace QCPL {

class AxisFormatDlgProps;
class PenEditorWidget;
class TextEditorWidget;

class AxisFormatWidget : public QWidget
{
    Q_OBJECT

public:
    AxisFormatWidget(QCPAxis* axis, const AxisFormatDlgProps &props);
    ~AxisFormatWidget();

public slots:
    void apply();
    void restore();

private:
    enum LabelsNumberFormat { lnfF, lnfE, lnfG };
    enum LabelsExpFormat { lefLowerE, lefUpperE, lefDot, lefCross };

    QCPAxis *_axis;
    QJsonObject _backup;
    QTabBar *_tabs;
    TextEditorWidget *_titleEditor, *_labelsEditor;
    QSpinBox *_outerMargin, *_innerMargin, *_offset, *_labelsAngle, *_labelsPadding,
        *_labelsPrecision, *_tickLengthIn, *_tickLengthOut, *_subTickLengthIn, *_subTickLengthOut;
    QCheckBox *_visible, *_saveDefault, *_logarithmic, *_reversed, *_labelsVisible, *_labelsInside;
    QGroupBox *_axisGroup, *_gridGroup, *_groupSubGrid, *_groupTicks, *_groupSubTicks;
    Ori::Widgets::OptionsGroup *_numberFormat, *_expFormat;
    PenEditorWidget *_axisPen, *_tickPen, *_subTickPen, *_gridPen, *_zeroPen, *_subGridPen;
};

} // namespace QCPL

#endif // QCPL_FORMAT_AXIS_H
