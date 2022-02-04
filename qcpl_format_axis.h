#ifndef QCPL_FORMAT_AXIS_H
#define QCPL_FORMAT_AXIS_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSpinBox;
QT_END_NAMESPACE

class QCPAxis;

namespace QCPL {

// TODO it's not clear what edit here
class AxisFormatWidget : public QWidget
{
    Q_OBJECT

public:
    AxisFormatWidget(QCPAxis* axis);

public slots:
    void apply();

private:
    QCPAxis *_axis;
    QSpinBox *_padding, *_labelPadding, *_offset;
};

} // namespace QCPL

#endif // QCPL_FORMAT_AXIS_H
