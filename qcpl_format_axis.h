#ifndef QCPL_FORMAT_AXIS_H
#define QCPL_FORMAT_AXIS_H

#include <QTabWidget>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QCheckBox;
QT_END_NAMESPACE

namespace Ori {
namespace Widgets {
class OptionsGroup;
}}

class QCPAxis;

namespace QCPL {

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
        *_labelsPrecision;
    QCheckBox *_logarithmic, *_reversed, *_labelsVisible, *_labelsInside;
    Ori::Widgets::OptionsGroup *_numberFormat, *_expFormat;
};

} // namespace QCPL

#endif // QCPL_FORMAT_AXIS_H
