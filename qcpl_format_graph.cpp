#include "qcpl_format_graph.h"

#include "qcpl_format_editors.h"
#include "qcpl_plot.h"

#include "widgets/OriLabels.h"
#include "helpers/OriLayouts.h"

using namespace Ori::Layouts;

namespace QCPL {

GraphFormatWidget::GraphFormatWidget(QCPGraph *graph) : QWidget(), _graph(graph)
{
    _linePen = new PenEditorWidget;
    _linePen->setValue(graph->pen());

    LayoutV({
                makeLabelSeparator(tr("Line")),
                _linePen,
                Stretch(),
            }).setMargin(0).useFor(this);
}

void GraphFormatWidget::apply()
{
    _graph->setPen(_linePen->value());
}

} // namespace QCPL

