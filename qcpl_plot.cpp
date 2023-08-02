#include "qcpl_plot.h"

#include "qcpl_colors.h"
#include "qcpl_graph.h"
#include "qcpl_format.h"
#include "qcpl_io_json.h"

/// Returns true when range is corrected, false when it's unchanged.
static bool correctZeroRange(QCPRange& range, double safeMargin)
{
    auto epsilon = std::numeric_limits<double>::epsilon();
    if (range.size() > epsilon) return false;

    // constant line at zero level
    if (qAbs(range.lower) <= epsilon)
    {
        range.lower = -1;
        range.upper = 1;
        return true;
    }

    // constant line at any level
    double delta = qAbs(range.lower) * safeMargin;
    range.lower -= delta;
    range.upper += delta;
    return true;
}

inline QFont defaultTitleFont() {
    return QFont("sans",
             #ifdef Q_OS_MAC
                 16,
             #else
                 14,
             #endif
                 QFont::Bold);
}

namespace QCPL {

Plot::Plot(QWidget *parent) : QCustomPlot(parent),
        // TODO: make configurable
        _safeMarginsX(1.0/100.0),
        _safeMarginsY(5.0/100.0),
        _zoomStepX(1.0/100.0),
        _zoomStepY(1.0/100.0),
        _numberPrecision(10)
{
    yAxis->setNumberPrecision(_numberPrecision);
    xAxis->setNumberPrecision(_numberPrecision);

    if (!LineGraph::sharedSelectionDecorator())
    {
        // TODO: make selector customizable: line color/width/visibility, points count/color/size/visibility
        auto decorator = new QCPSelectionDecorator;
        decorator->setPen(QPen(QBrush(QColor(0, 255, 255, 120)), 2));
        decorator->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, Qt::black, Qt::black, 6));
        decorator->setUsedScatterProperties(QCPScatterStyle::spAll);
        LineGraph::setSharedSelectionDecorator(decorator);
    }

    legend->setVisible(true);

    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables |
                    QCP::iSelectAxes | QCP::iSelectItems | QCP::iSelectLegend | QCP::iSelectOther);
    connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(plotSelectionChanged()));
    connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)),
            this, SLOT(rawGraphClicked(QCPAbstractPlottable*)));
    connect(this, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)),
            this, SLOT(axisDoubleClicked(QCPAxis*,QCPAxis::SelectablePart)));

    auto f = font();
#ifdef Q_OS_MAC
    f.setPointSize(14);
#else
    f.setPointSize(10);
#endif
    xAxis->setLabelFont(f);
    yAxis->setLabelFont(f);
    xAxis->setSelectedLabelFont(f);
    yAxis->setSelectedLabelFont(f);
}

Plot::~Plot()
{
    auto it = _formatters.constBegin();
    while (it != _formatters.constEnd())
    {
        delete it.value();
        it++;
    }
}

Plot::PlotPart Plot::selectedPart() const
{
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        return PlotPart::AxisX;

    if (yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        return PlotPart::AxisY;

    return PlotPart::None;
}

void Plot::mouseDoubleClickEvent(QMouseEvent *event)
{
    QCustomPlot::mouseDoubleClickEvent(event);

    QCPLayerable *selectedLayerable = layerableAt(event->pos(), true);
    if (!selectedLayerable)
        emit emptySpaceDoubleClicked(event);
}

void Plot::mousePressEvent(QMouseEvent *event)
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged
    switch (selectedPart()) {
    case PlotPart::AxisX:
        axisRect()->setRangeDrag(xAxis->orientation());
        break;
    case PlotPart::AxisY:
        axisRect()->setRangeDrag(yAxis->orientation());
        break;
    default:
        axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    }
    QCustomPlot::mousePressEvent(event);
}

void Plot::wheelEvent(QWheelEvent *event)
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed
    switch (selectedPart()) {
    case PlotPart::AxisX:
        axisRect()->setRangeZoom(xAxis->orientation());
        break;
    case PlotPart::AxisY:
        axisRect()->setRangeZoom(yAxis->orientation());
        break;
    default:
        axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    }
    QCustomPlot::wheelEvent(event);
}

void Plot::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu* menu = nullptr;
    QPointF pos(event->x(), event->y());
    if (legend->selectTest(pos, true) >= 0)
        menu = menuLegend;
    else if (xAxis->getPartAt(pos) != QCPAxis::spNone)
        menu = menuAxisX;
    else if (yAxis->getPartAt(pos) != QCPAxis::spNone)
        menu = menuAxisY;
    else if (menuGraph) {
        foreach (auto g, selectedGraphs())
            if (!isService(g)) {
                menu = menuGraph;
                break;
            }
    }
    if (!menu) menu = menuPlot;
    if (menu) menu->popup(event->globalPos());
}

void Plot::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
    emit resized(event->oldSize(), event->size());
}

void Plot::plotSelectionChanged()
{
    // handle axis and tick labels as one selectable object:
    if (xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        xAxis->selectedParts().testFlag(QCPAxis::spTickLabels))
        xAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
    if (yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        yAxis->selectedParts().testFlag(QCPAxis::spTickLabels))
        yAxis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
}

void Plot::rawGraphClicked(QCPAbstractPlottable *plottable)
{
    auto g = dynamic_cast<QCPGraph*>(plottable);
    if (_serviceGraphs.contains(g)) g = nullptr;
    emit graphClicked(g);
}

void Plot::axisDoubleClicked(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    if (part == QCPAxis::spAxisLabel)
        titleDlg(axis);
    else
        limitsDlg(axis);
}

void Plot::autolimits(QCPAxis* axis, bool replot)
{
    QCPRange totalRange;
    bool isTotalValid = false;
    for (int i = 0; i < graphCount(); i++)
    {
        auto g = graph(i);

        if (!g->visible()) continue;

        if (excludeServiceGraphsFromAutolimiting)
            if (_serviceGraphs.contains(g))
                continue;

        bool hasRange = false;
        auto range = axis == xAxis
                ? g->getKeyRange(hasRange, QCP::sdBoth)
                : g->getValueRange(hasRange, QCP::sdBoth, QCPRange());
        if (!hasRange) continue;

        if (!isTotalValid)
        {
            totalRange = range;
            isTotalValid = true;
        }
        else totalRange.expand(range);
    }

    if (!isTotalValid) return;

    bool corrected = correctZeroRange(totalRange, safeMargins(axis));
    axis->setRange(totalRange);

    if (!corrected && useSafeMargins)
        extendLimits(axis, safeMargins(axis), false);

    if (replot) this->replot();
}

void Plot::extendLimits(double factor, bool replot)
{
    extendLimits(xAxis, factor, false);
    extendLimits(yAxis, factor, replot);
}

void Plot::extendLimits(QCPAxis* axis, double factor, bool replot)
{
    auto range = axis->range();
    auto delta = (range.upper - range.lower) * factor;
    range.upper += delta;
    range.lower -= delta;
    setAxisRange(axis, range);
    if (replot) this->replot();
}

void Plot::setLimits(QCPAxis* axis, double min, double max, bool replot)
{
    QCPRange range(min, max);
    range.normalize();
    setAxisRange(axis, range);
    if (replot) this->replot();
}

AxisLimits Plot::limits(QCPAxis* axis) const
{
    auto range = axis->range();
    return AxisLimits(range.lower, range.upper);
}

double Plot::safeMargins(QCPAxis* axis)
{
    return axis == xAxis ? _safeMarginsX : _safeMarginsY;
}

void Plot::setAxisRange(QCPAxis* axis, const QCPRange& range)
{
    QCPRange r = range;
    correctZeroRange(r, safeMargins(axis));
    axis->setRange(r);
}

bool Plot::limitsDlg(QCPAxis* axis)
{
    auto range = axis->range();
    AxisLimitsDlgProps props;
    props.title = tr("%1 Limits").arg(getAxisIdent(axis));
    props.precision = _numberPrecision;
    props.unit = getAxisUnitString ? getAxisUnitString(axis) : QString();
    if (axisLimitsDlg(range, props))
    {
        setAxisRange(axis, range);
        replot();
        return true;
    }
    return false;
}

bool Plot::limitsDlgXY()
{
    auto range = (selectedPart() == PlotPart::AxisY ? yAxis : xAxis)->range();
    AxisLimitsDlgProps props;
    props.title = tr("Limits for X and Y");
    props.precision = _numberPrecision;
    if (axisLimitsDlg(range, props))
    {
        setAxisRange(xAxis, range);
        setAxisRange(yAxis, range);
        replot();
        return true;
    }
    return false;
}

bool Plot::titleDlg(QCPAxis* axis)
{
    AxisTitleDlgPropsV2 props;
    props.title = tr("%1 Title").arg(getAxisIdent(axis));
    props.formatter = _formatters.contains(axis) ? _formatters[axis] : nullptr;
    props.defaultTitle = defaultTitle(axis);
    if (axisTitleDlgV2(axis, props))
    {
        replot();
        emit modified(QStringLiteral("%1 changed").arg(props.title));
        return true;
    }
    return false;
}

bool Plot::formatDlg(QCPAxis* axis)
{
    AxisFormatDlgProps props;
    props.title = tr("%1 Format").arg(getAxisIdent(axis));
    props.plot = this;
    if (axisFormatDlg(axis, props))
    {
        replot();
        return true;
    }
    return false;
}

bool Plot::formatDlg0()
{
    TitleFormatDlgProps props;
    props.title = tr("Diagram title");
    props.plot = this;

    bool oldVisible = _title;
    if (!oldVisible)
        setTitleVisible(true);

    if (titleFormatDlg(_title, props))
    {
        replot();
        return true;
    }

    if (!oldVisible) setTitleVisible(false);
    return false;
}

bool Plot::formatDlgLegend()
{
    LegendFormatDlgProps props;
    props.title = tr("Legend");
    if (formatSaver)
        props.onSaveDefault = [this](){ formatSaver->saveLegend(legend); };

    if (legendFormatDlg(legend, props))
    {
        replot();
        return true;
    }
    return false;
}

QString Plot::getAxisIdent(QCPAxis* axis) const
{
   if (axis == xAxis)
       return tr("X-axis");
   if (axis == yAxis)
       return tr("Y-axis");
   auto label = axis->label();
   return label.isEmpty() ? tr("Axis") : label;
}

bool Plot::isFrameVisible() const
{
    return xAxis2->visible();
}

void Plot::setFrameVisible(bool on)
{
    // Secondary axes only form frame rect
    xAxis2->setVisible(on);
    yAxis2->setVisible(on);
    xAxis2->setTicks(false);
    yAxis2->setTicks(false);
    xAxis2->setSelectableParts({});
    yAxis2->setSelectableParts({});
}

Graph* Plot::makeNewGraph(const QString& title)
{
    auto g = new LineGraph(xAxis, yAxis);

    if (graphAutoColors)
        g->setPen(nextGraphColor());

    g->setName(title);
    g->setSelectable(_selectionType);
    return g;
}

Graph* Plot::makeNewGraph(const QString &title, const GraphData &data, bool replot)
{
    auto g = makeNewGraph(title);
    g->setData(data.x, data.y);
    if (replot) this->replot();
    return g;
}

void Plot::updateGraph(Graph* graph, const GraphData &data, bool replot)
{
    graph->setData(data.x, data.y);
    if (replot) this->replot();
}

QColor Plot::nextGraphColor()
{
    if (_nextColorIndex == defaultColorSet().size())
        _nextColorIndex = 0;
    return defaultColorSet().at(_nextColorIndex++);
}

void Plot::setTitleVisible(bool on)
{
    if (_title && on) return;
    if (on)
    {
        _title = new QCPTextElement(this);
        _title->setMargins({10, 10, 10, 0});
        _title->setSelectable(true);

        // Restore settings
        if (_backup.contains("title_color"))
            _title->setTextColor(_backup["title_color"].toString());
        if (_backup.contains("title_text"))
            _title->setText(_backup["title_text"].toString());
        QFont f = defaultTitleFont();
        if (_backup.contains("title_font"))
            f.fromString(_backup["title_font"].toString());
        _title->setFont(f);
        _title->setSelectedFont(f);
        if (_backup.contains("title_flags"))
            _title->setTextFlags(_backup["title_flags"].toInt());

        plotLayout()->insertRow(0);
        plotLayout()->addElement(0, 0, _title);
        connect(_title, &QCPTextElement::doubleClicked, this, [this](){ emit editTitleRequest(); });
    }
    else
    {
        // Store settings
        _backup["title_text"] = _title->text();
        _backup["title_color"] = _title->textColor().name();
        _backup["title_font"] = _title->font().toString();
        _backup["title_flags"] = _title->textFlags();

        plotLayout()->remove(_title);
        plotLayout()->simplify();
        _title = nullptr;
    }
}

Graph* Plot::selectedGraph() const
{
    auto graphs = selectedGraphs();
    return graphs.isEmpty() ? nullptr : graphs.first();
}

void Plot::copyPlotImage()
{
    QImage image(width(), height(), QImage::Format_RGB32);
    QCPPainter painter(&image);
    toPainter(&painter);
    qApp->clipboard()->setImage(image);
}

void Plot::addFormatter(void* target, TextFormatterBase* formatter)
{
    if (_formatters.contains(target))
        qWarning() << "Formatter is already registerd for this target, it will be lost (possible memory leak)";
    _formatters[target] = formatter;
}

void Plot::addTextVar(void* target, const QString& name, const QString& descr, TextVarGetter getter)
{
    if (!_formatters.contains(target))
    {
        if (target == xAxis)
            _formatters[target] = new AxisTitleFormatter(xAxis);
        else if (target == yAxis)
            _formatters[target] = new AxisTitleFormatter(yAxis);
        else
            return;
    }
    _formatters[target]->addVar(name, descr, getter);
}

void Plot::updateTitles()
{
    auto it = _formatters.constBegin();
    while (it != _formatters.constEnd())
    {
        it.value()->format();
        it++;
    }
}

void Plot::updateTitle(void* target)
{
    auto fmt = formatter(target);
    if (fmt) fmt->format();
}

void Plot::setFormatterText(void* target, const QString& text)
{
    auto fmt = formatter(target);
    if (fmt) fmt->setText(text);
}

QString Plot::formatterText(void* target) const
{
    auto fmt = formatter(target);
    return fmt ? fmt->text() : QString();
}

} // namespace QCPL

