#include "qcpl_format_graph.h"

#include "qcpl_format_editors.h"
#include "qcpl_io_json.h"
#include "qcustomplot/qcustomplot.h"

#include "helpers/OriLayouts.h"
#include "widgets/OriMenuToolButton.h"
#include "widgets/OriColorSelectors.h"

#include <QGridLayout>
#include <QSpinBox>
#include <QToolButton>

using namespace Ori::Layouts;

namespace QCPL {

GraphFormatWidget::GraphFormatWidget(QCPGraph *graph) : QWidget(), _graph(graph)
{
    _backup = writeGraph(graph);

    _linePen = new PenEditorWidget;

    _markerShape = new Ori::Widgets::MenuToolButton;
    _markerShape->setIconSize({PenEditorWidget::penIconWidthNormal, PenEditorWidget::iconHeight});
    createMarkerShapeAction(QCPScatterStyle::ssNone, tr("None"));
    // This is a tiny dot and the same effect can be achived with Circle of size 1
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
    createMarkerShapeAction(QCPScatterStyle::ssPeace, tr("Peace"));

    _markerSize = new QSpinBox;

    _markerColor = new Ori::Widgets::ColorButton;
    _markerColor->drawIconFrame = false;
    _markerColor->setIconSize({40, 16});

    auto markerLayout = new QGridLayout;
    markerLayout->setHorizontalSpacing(10);
    markerLayout->addWidget(new QLabel(tr("Shape:")), 0, 0);
    markerLayout->addWidget(_markerShape, 0, 1);
    markerLayout->addWidget(new QLabel(tr("Size:")), 0, 2);
    markerLayout->addWidget(_markerSize, 0, 3);
    markerLayout->addWidget(new QLabel(tr("Color:")), 0, 4);
    markerLayout->addWidget(_markerColor, 0, 5);

    PenEditorWidgetOptions opts;
    opts.labelStyle = tr("Pen:");
    opts.gridLayout = markerLayout;
    opts.gridRow = 1;
    // we don't put _markerPen widget itself into a layout,
    // so a parent must be provided to make it freed
    _markerPen = new PenEditorWidget(opts, this);

    _markerSkip = new QSpinBox;
    _markerSkip->setMinimum(1);
    _markerSkip->setMaximum(1000000);

    LayoutV({
                makeLabelSeparator(tr("Line")),
                _linePen,
                Space(10),
                makeLabelSeparator(tr("Markers")),
                markerLayout,
                Space(10),
                LayoutH({
                    new QLabel(tr("Drawn marker each n-th point:")),
                    _markerSkip,
                    Stretch(),
                }),
                makeLabelSeparator(""),
            }).useFor(this);
}

static QPixmap makeScatterShapeIcon(int shape, const QSize& sz)
{
    if (shape == QCPScatterStyle::ssNone)
        return QIcon(":/qcpl_images/style_empty").pixmap(sz);

    QPixmap pixmap(sz.height(), sz.height());
    pixmap.fill(Qt::transparent);
    QCPPainter p(&pixmap);
    p.setAntialiasing(true);

    QCPScatterStyle ss;
    ss.setShape(QCPScatterStyle::ScatterShape(shape));
    ss.setPen(QPen(Qt::black));
    ss.setSize(sz.height()-2);
    ss.drawShape(&p, sz.height()/2, sz.height()/2);

    return pixmap;
}

void GraphFormatWidget::createMarkerShapeAction(int shape, const QString& title)
{
    auto icon = makeScatterShapeIcon(shape, _markerShape->iconSize());
    _markerShape->addAction(shape, new QAction(icon, title, this));
}

void GraphFormatWidget::populate()
{
    _linePen->setValue(_graph->pen());
    QCPScatterStyle scatterStyle = _graph->scatterStyle();
    _markerShape->setSelectedId(scatterStyle.shape());
    _markerSize->setValue(scatterStyle.size());
    _markerColor->setSelectedColor(scatterStyle.brush().color());
    _markerPen->setValue(_graph->scatterStyle().pen());
    _markerSkip->setValue(_graph->scatterSkip()+1);
}

void GraphFormatWidget::restore()
{
    readGraph(_backup, _graph);
    _graph->parentPlot()->replot();
}

void GraphFormatWidget::apply()
{
    _graph->setPen(_linePen->value());
    QCPScatterStyle scatterStyle = _graph->scatterStyle();
    scatterStyle.setShape(QCPScatterStyle::ScatterShape(_markerShape->selectedId()));
    scatterStyle.setSize(_markerSize->value());
    scatterStyle.setBrush(_markerColor->selectedColor());
    auto markerPen = _markerPen->value();
    switch (scatterStyle.shape())
    {
    case QCPScatterStyle::ssCross:
    case QCPScatterStyle::ssPlus:
    case QCPScatterStyle::ssStar:
    case QCPScatterStyle::ssPeace:
        // There markers are invisible if there is not pen and this can be confusing
        // also it doesn't make any sence to select them without pen
        if (markerPen.style() == Qt::PenStyle::NoPen)
            markerPen.setStyle(Qt::PenStyle::SolidLine);
        break;
    default:
        break;
    }
    scatterStyle.setPen(markerPen);
    _graph->setScatterStyle(scatterStyle);
    _graph->setScatterSkip(_markerSkip->value()-1);
    _graph->parentPlot()->replot();
}

} // namespace QCPL

