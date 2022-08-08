#include "qcpl_format_graph.h"

#include "qcpl_format_editors.h"
#include "qcpl_plot.h"

#include "helpers/OriLayouts.h"
#include "widgets/OriLabels.h"
#include "widgets/OriMenuToolButton.h"

#include <QGridLayout>
#include <QSpinBox>
#include <QToolButton>

using namespace Ori::Layouts;

namespace QCPL {

GraphFormatWidget::GraphFormatWidget(QCPGraph *graph) : QWidget(), _graph(graph)
{
    _linePen = new PenEditorWidget;
    _linePen->setValue(graph->pen());

    QCPScatterStyle scatterStyle = _graph->scatterStyle();

    _markerShape = new Ori::Widgets::MenuToolButton;
    _markerShape->setIconSize({16, 16});
    createMarkerShapeAction(QCPScatterStyle::ssNone, tr("None"));
    //createMarkerShapeAction(QCPScatterStyle::ssDot, tr("Dot"));
    createMarkerShapeAction(QCPScatterStyle::ssCross, tr("Cross"));
    createMarkerShapeAction(QCPScatterStyle::ssPlus, tr("Plus"));
    createMarkerShapeAction(QCPScatterStyle::ssCircle, tr("Circle"));
    createMarkerShapeAction(QCPScatterStyle::ssDisc, tr("Disc"));
    createMarkerShapeAction(QCPScatterStyle::ssSquare, tr("Square"));
    createMarkerShapeAction(QCPScatterStyle::ssDiamond, tr("Diamond"));
    createMarkerShapeAction(QCPScatterStyle::ssStar, tr("Star"));
    createMarkerShapeAction(QCPScatterStyle::ssTriangle, tr("Triangle 1"));
    createMarkerShapeAction(QCPScatterStyle::ssTriangleInverted, tr("Triangle 2"));
    createMarkerShapeAction(QCPScatterStyle::ssCrossSquare, tr("Cross-square"));
    createMarkerShapeAction(QCPScatterStyle::ssPlusSquare, tr("Plus-square"));
    createMarkerShapeAction(QCPScatterStyle::ssCrossCircle, tr("Cross-circle"));
    createMarkerShapeAction(QCPScatterStyle::ssPlusCircle, tr("Plus-circle"));
    _markerShape->setSelectedId(scatterStyle.shape());

    _markerSize = new QSpinBox;
    _markerSize->setValue(scatterStyle.size());

    _markerColor = new ColorButton;
    _markerColor->setIconSize({40, 16});
    _markerColor->setValue(scatterStyle.brush().color());

    auto markerLayout = new QGridLayout;
    markerLayout->setHorizontalSpacing(10);
    markerLayout->addWidget(new QLabel(tr("Shape:")), 0, 0);
    markerLayout->addWidget(_markerShape, 0, 1);
    markerLayout->addWidget(new QLabel(tr("Size:")), 0, 2);
    markerLayout->addWidget(_markerSize, 0, 3);
    markerLayout->addWidget(new QLabel(tr("Color:")), 0, 4);
    markerLayout->addWidget(_markerColor, 0, 5);

    PenEditorWidgetOptions opts;
    opts.labelStyle = tr("Lines:");
    opts.gridLayout = markerLayout;
    opts.gridRow = 1;
    // we don't put _markerPen widget itself into a layout,
    // so a parent must be provided to make it freed
    _markerPen = new PenEditorWidget(opts, this);
    _markerPen->setValue(graph->scatterStyle().pen());

    _markerSkip = new QSpinBox;
    _markerSkip->setValue(graph->scatterSkip());

    LayoutV({
                makeLabelSeparator(tr("Line")),
                _linePen,
                Space(10),
                makeLabelSeparator(tr("Markers")),
                markerLayout,
                Space(10),
                LayoutH({
                    new QLabel(tr("Skip points after each drawn marker:")),
                    _markerSkip,
                    Stretch(),
                }),
                makeLabelSeparator(""),
                Space(20),
                Stretch(),
            }).setMargin(0).useFor(this);
}

static QPixmap makeScatterShapeIcon(int shape, const QSize& sz)
{
    if (shape == QCPScatterStyle::ssNone)
        return QIcon(":/qcpl_images/style_empty").pixmap(sz);

    QPixmap pixmap(sz);
    pixmap.fill(Qt::transparent);
    QCPPainter p(&pixmap);
    p.setAntialiasing(true);

    QCPScatterStyle ss;
    ss.setShape(QCPScatterStyle::ScatterShape(shape));
    ss.setPen(QPen(Qt::black));
    ss.setSize(sz.height()-2);
    ss.drawShape(&p, sz.width()/2, sz.height()/2);

    return pixmap;
}

void GraphFormatWidget::createMarkerShapeAction(int shape, const QString& title)
{
    _markerShape->addAction(shape, new QAction(makeScatterShapeIcon(shape, _markerShape->iconSize()), title, this));
}

void GraphFormatWidget::apply()
{
    _graph->setPen(_linePen->value());
    QCPScatterStyle scatterStyle = _graph->scatterStyle();
    scatterStyle.setShape(QCPScatterStyle::ScatterShape(_markerShape->selectedId()));
    scatterStyle.setSize(_markerSize->value());
    scatterStyle.setBrush(_markerColor->value());
    scatterStyle.setPen(_markerPen->value());
    _graph->setScatterStyle(scatterStyle);
    _graph->setScatterSkip(_markerSkip->value());
}

} // namespace QCPL

