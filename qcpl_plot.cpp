#include "qcpl_plot.h"

#include "qcpl_axis_ticker.h"
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

    _title = new QCPTextElement(this);
    _title->setMargins({10, 10, 10, 10});
    _title->setSelectable(true);
    _title->setVisible(false);
    plotLayout()->insertRow(0);
    plotLayout()->setRowSpacing(0),
    plotLayout()->setRowStretchFactor(0, 0.01);
    connect(_title, &QCPTextElement::doubleClicked, this, [this](){ titleTextDlg(); });

    _backupLayout = new QCPLayoutGrid;
    _backupLayout->setVisible(false);
    _backupLayout->addElement(0, 0, _title);

    legend->setVisible(true);

    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables |
                    QCP::iSelectAxes | QCP::iSelectItems | QCP::iSelectLegend | QCP::iSelectOther);
    connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(plotSelectionChanged()));
    connect(this, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)),
            this, SLOT(rawGraphClicked(QCPAbstractPlottable*)));
    connect(this, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)),
            this, SLOT(axisDoubleClicked(QCPAxis*,QCPAxis::SelectablePart)));

    auto labelsFont = font();
    auto titleFont = _title->font();
#ifdef Q_OS_MAC
    labelsFont.setPointSize(14);
    titleFont.setPointSize(16);
#else
    labelsFont.setPointSize(10);
    titleFont.setPointSize(14);
#endif
    xAxis->setLabelFont(labelsFont);
    yAxis->setLabelFont(labelsFont);
    xAxis->setSelectedLabelFont(labelsFont);
    yAxis->setSelectedLabelFont(labelsFont);
    _title->setFont(titleFont);
    _title->setSelectedFont(titleFont);
}

Plot::~Plot()
{
    delete _backupLayout;
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
    QPointF pos(event->x(), event->y());
    QMenu* menu = nullptr;
    if (menuTitle && _title->selectTest(pos, false) >= 0) menu = menuTitle;
    else if (menuLegend && legend->selectTest(pos, false) >= 0) menu = menuLegend;
    else if (menuAxisX && xAxis->selectTest(pos, false) >= 0) menu = menuAxisX;
    else if (menuAxisY && yAxis->selectTest(pos, false) >= 0) menu = menuAxisY;
    if (!menu && menuGraph) {
        foreach (auto g, selectedGraphs())
            if (!isService(g)) {
                menu = menuGraph;
                break;
            }
    }
    if (!menu) {
        auto it = menus.constBegin();
        while (it != menus.constEnd()) {
            if (it.key()->selectTest(pos, false) > 0) {
                menu = it.value();
                break;
            }
            it++;
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
        axisTextDlg(axis);
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

bool Plot::axisTextDlg(QCPAxis* axis)
{
    AxisFormatDlgProps props;
    props.title = tr("%1 Title").arg(getAxisIdent(axis));
    props.formatter = formatter(axis);
    props.defaultText = defaultText(axis);
    if (QCPL::axisTextDlg(axis, props))
    {
        emit modified("Plot::axisTextDlg");
        return true;
    }
    return false;
}

bool Plot::axisFormatDlg(QCPAxis* axis)
{
    AxisFormatDlgProps props;
    props.title = tr("%1 Format").arg(getAxisIdent(axis));
    props.formatter = formatter(axis);
    props.defaultText = defaultText(axis);
    if (formatSaver)
        props.onSaveDefault = [this, axis ](){ formatSaver->saveAxis(axis); };
    if (QCPL::axisFormatDlg(axis, props))
    {
        emit modified("Plot::axisFormatDlg");
        return true;
    }
    return false;
}

bool Plot::colorScaleFormatDlg(QCPColorScale* scale)
{
    AxisFormatDlgProps props;
    props.title = tr("%1 Format").arg(getAxisIdent(scale->axis()));
    props.formatter = formatter(scale->axis());
    props.defaultText = defaultText(scale->axis());
    if (formatSaver)
        props.onSaveDefault = [this, scale](){ formatSaver->saveColorScale(scale); };
    if (QCPL::colorScaleFormatDlg(scale, props))
    {
        emit modified("Plot::colorScaleFormatDlg");
        return true;
    }
    return false;
}

bool Plot::titleTextDlg()
{
    TitleFormatDlgProps props;
    props.title = tr("Title Text");
    props.formatter = formatter(_title);
    props.defaultText = defaultText(_title);
    if (QCPL::titleTextDlg(_title, props))
    {
        emit modified("Plot::titleTextDlg");
        return true;
    }
    return false;
}

bool Plot::titleFormatDlg()
{
    TitleFormatDlgProps props;
    props.title = tr("Title Format");
    props.formatter = formatter(_title);
    props.defaultText = defaultText(_title);
    if (formatSaver)
        props.onSaveDefault = [this](){ formatSaver->saveTitle(_title); };
    if (QCPL::titleFormatDlg(_title, props))
    {
        emit modified("Plot::titleFormatDlg");
        return true;
    }
    return false;
}

bool Plot::legendFormatDlg()
{
    LegendFormatDlgProps props;
    props.title = tr("Legend Format");
    if (formatSaver)
        props.onSaveDefault = [this](){ formatSaver->saveLegend(legend); };
    if (QCPL::legendFormatDlg(legend, props))
    {
        emit modified("Plot::legendFormatDlg");
        return true;
    }
    return false;
}

static QCPAxis* getOppositeAxis(QCPAxis* axis)
{
    auto rect = axis->axisRect();
    auto type = axis->axisType();
    if (type == QCPAxis::atLeft)
        return rect->axis(QCPAxis::atRight);
    if (type == QCPAxis::atRight)
        return rect->axis(QCPAxis::atLeft);
    if (type == QCPAxis::atTop)
        return rect->axis(QCPAxis::atBottom);
    return rect->axis(QCPAxis::atTop);
}

QString Plot::getAxisIdent(QCPAxis* axis) const
{
    if (axisIdents.contains(axis))
        return axisIdents[axis];
    auto axis2 = getOppositeAxis(axis);
    if (axisIdents.contains(axis2))
        return axisIdents[axis2];
    if (axis == xAxis || axis == xAxis2)
        return tr("X-axis");
    if (axis == yAxis || axis == yAxis2)
        return tr("Y-axis");
    if (!axis->label().isEmpty())
        return axis->label();
    return tr("Axis");
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
            _formatters[target] = new AxisTextFormatter(xAxis);
        else if (target == yAxis)
            _formatters[target] = new AxisTextFormatter(yAxis);
        else if (target == _title)
            _formatters[target] = new TitleTextFormatter(_title);
        else
            return;
    }
    _formatters[target]->addVar(name, descr, getter);
}

void Plot::updateTexts()
{
    auto it = _formatters.constBegin();
    while (it != _formatters.constEnd())
    {
        it.value()->format();
        it++;
    }
}

void Plot::updateText(void* target)
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

void Plot::updateTitleVisibility()
{
    // We can't just hide the title, because the layout will still respect it's size.
    // We can't just extract the title from layout because it will be deleted.
    // So we have to move it into another layout instead.
    QString text;
    if (auto f = formatter(_title); f)
        text = f->text();
    else text = _title->text();
    if (!_title->visible() or text.isEmpty())
    {
        // it's ok to get element without checking, but a console warning is printed then
        if (_backupLayout->hasElement(0, 0) and _backupLayout->element(0, 0) == _title)
            return;
        _backupLayout->addElement(0, 0, _title);
    }
    else
    {
        auto mainLayout = plotLayout();
        auto p = titleRC();
        if (mainLayout->hasElement(p.row, p.col) and mainLayout->element(p.row, p.col) == _title)
            return;
        mainLayout->addElement(p.row, p.col, _title);
    }
}

int Plot::graphsCount(GraphCountFlags flags) const
{
    int count = 0;
    for (int i = 0; i < graphCount(); i++)
    {
        auto g = graph(i);
        if (!g->visible() and (flags & COUNT_ONLY_VISIBLE))
            continue;
        if (isService(g) and !(flags & COUNT_SERVICE))
            continue;
        count++;
    }
    return count;
}

AxisFactor Plot::axisFactor(QCPAxis* axis) const
{
    auto factorTicker = dynamic_cast<FactorAxisTicker*>(axis->ticker().data());
    return factorTicker ? factorTicker->factor : AxisFactor();
}

void Plot::setAxisFactor(QCPAxis* axis, const AxisFactor& factor)
{
    auto factorTicker = dynamic_cast<FactorAxisTicker*>(axis->ticker().data());
    if (factorTicker)
    {
        if (isAxisFactorSet(factor))
            factorTicker->factor = factor;
        else
        {
            qDebug() << "Reset axis factor";
            axis->setTicker(factorTicker->prevTicker);
        }
    }
    else if (isAxisFactorSet(factor))
    {
        auto factorTicker = new FactorAxisTicker(axis->ticker());
        factorTicker->factor = factor;
        axis->setTicker(QSharedPointer<QCPAxisTicker>(factorTicker));
    }
    replot();
}

} // namespace QCPL
