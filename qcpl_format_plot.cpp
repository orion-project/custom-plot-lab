#include "qcpl_format_plot.h"
#include "qcpl_plot.h"

#include "helpers/OriLayouts.h"

#include <QDebug>
#include <QLabel>
#include <QSpinBox>

using namespace Ori::Layouts;

namespace QCPL {

PlotFormatWidget::PlotFormatWidget(Plot* plot, const PlotFormatSettings &format) : QWidget(), _plot(plot), _format(format)
{
    // TODO
}

void PlotFormatWidget::apply()
{
    // TODO
}

} // namespace QCPL
