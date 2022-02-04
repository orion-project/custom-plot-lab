#include "qcpl_format_axis.h"
#include "qcpl_plot.h"

#include "helpers/OriLayouts.h"

#include <QDebug>
#include <QLabel>
#include <QSpinBox>

using namespace Ori::Layouts;

namespace QCPL {

AxisFormatWidget::AxisFormatWidget(QCPAxis* axis) : QWidget(), _axis(axis)
{
    _padding = new QSpinBox; _padding->setValue(_axis->padding());
    _labelPadding = new QSpinBox; _labelPadding->setValue(_axis->labelPadding());
    _offset = new QSpinBox; _offset->setValue(_axis->offset());

    LayoutV({
                LayoutH({new QLabel(tr("Padding")), _padding}),
                LayoutH({new QLabel(tr("Label Padding")), _labelPadding}),
                LayoutH({new QLabel(tr("Offset")), _offset}),
    }).useFor(this);
}

void AxisFormatWidget::apply()
{
    _axis->setPadding(_padding->value());
    _axis->setLabelPadding(_labelPadding->value());
    _axis->setOffset(_offset->value());
}

} // namespace QCPL
