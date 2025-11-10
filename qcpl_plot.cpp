#include "qcpl_plot.h"

#include "qcpl_axis.h"
#include "qcpl_axis_factor.h"
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

Plot::Plot(const PlotOptions& opts, QWidget *parent) : QCustomPlot(parent),
        // TODO: make configurable
        _safeMarginsX(1.0/100.0),
        _safeMarginsY(5.0/100.0),
        _zoomStepX(1.0/100.0),
        _zoomStepY(1.0/100.0),
        _numberPrecision(10)
{
    foreach (auto axis, defaultAxes())
    {
        initDefault(axis);
        if (opts.replaceDefaultAxes)
        {
            auto fmt = writeAxis(axis);
            auto axisType = axis->axisType();
            axisRect()->removeAxis(axis);
            readAxis(fmt, addAxis(axisType));
        }
    }

    if (!LineGraph::sharedSelectionDecorator())
    {
        // TODO: make selector customizable: line color/width/visibility, points count/color/size/visibility
        auto decorator = new QCPSelectionDecorator;
        decorator->setPen(QPen(QBrush(QColor(0, 240, 255, 120)), 2));
        //decorator->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, Qt::black, Qt::black, 6));
        //decorator->setUsedScatterProperties(QCPScatterStyle::spAll);
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

    auto titleFont = _title->font();
#ifdef Q_OS_MAC
    titleFont.setPointSize(16);
#else
    titleFont.setPointSize(14);
#endif
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

QCPAxis* Plot::selectedAxis() const
{
    foreach (auto axis, axisRect()->axes())
        if (axis->selectedParts().testFlag(QCPAxis::spAxis))
            return axis;
    return nullptr;
}

void Plot::mouseDoubleClickEvent(QMouseEvent *event)
{
    QCustomPlot::mouseDoubleClickEvent(event);

    QCPLayerable *selectedLayerable = layerableAt(event->pos(), true);
    if (!selectedLayerable)
        emit emptySpaceDoubleClicked(event);
}

QMenu* Plot::findContextMenu(const QPointF& pos)
{
    if (menuTitle && _title->selectTest(pos, false) >= 0)
        return menuTitle;
    if (menuLegend && legend->selectTest(pos, false) >= 0)
        return menuLegend;
    if (menuAxis) {
        foreach (auto axis, axisRect()->axes()) {
            if (axis->selectTest(pos, false) >= 0) {
                axisUnderMenu = axis;
                return menuAxis;
            }
        }
    }
    if (menuAxisX && xAxis->selectTest(pos, false) >= 0) {
        axisUnderMenu = xAxis;
        return menuAxisX;
    }
    if (menuAxisY && yAxis->selectTest(pos, false) >= 0) {
        axisUnderMenu = yAxis;
        return menuAxisY;
    }
    if (menuGraph) {
        foreach (auto g, selectedGraphs())
            if (!isService(g))
                return menuGraph;
    }
    auto it = menus.constBegin();
    while (it != menus.constEnd()) {
        if (it.key()->selectTest(pos, false) > 0)
            return it.value();
        it++;
    }
    return menuPlot;
}

void Plot::contextMenuEvent(QContextMenuEvent *event)
{
    auto menu = findContextMenu(event->pos());
    if (menu) menu->popup(event->globalPos());
}

void Plot::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
    emit resized(event->oldSize(), event->size());
}

void Plot::plotSelectionChanged()
{
    auto allAxes = axisRect()->axes();
    int countX = 0, countY = 0;
    bool axisSelected = false;
    for (auto axis : std::as_const(allAxes))
    {
        if (!axis->visible()) continue;

        if (axis->orientation() == Qt::Horizontal)
            countX++;
        else countY++;

        if (highlightAxesOfSelectedGraphs)
            if (auto a = dynamic_cast<Axis*>(axis); a)
                a->setHightlight(false);

        if (axis->selectedParts().testFlag(QCPAxis::spAxis) ||
            axis->selectedParts().testFlag(QCPAxis::spTickLabels))
        {
            axis->setSelectedParts(QCPAxis::spAxis | QCPAxis::spTickLabels);
            axisRect()->setRangeDragAxes({axis});
            axisRect()->setRangeZoomAxes({axis});
            axisSelected = true;
        }
    }
    if (axisSelected)
        return;

    QList<QCPAxis*> graphAxes;
    for (auto graph : std::as_const(mGraphs))
        if (graph->selected())
            graphAxes << graph->keyAxis() << graph->valueAxis();

    if (graphAxes.empty()) {
        axisRect()->setRangeDragAxes(allAxes);
        axisRect()->setRangeZoomAxes(allAxes);
        return;
    }

    if (highlightAxesOfSelectedGraphs)
        for (auto axis : std::as_const(graphAxes))
            if (auto a = dynamic_cast<Axis*>(axis); a)
                if ((a->isX() && countX > 1) || (a->isY() && countY > 1))
                    a->setHightlight(true);

    axisRect()->setRangeDragAxes(graphAxes);
    axisRect()->setRangeZoomAxes(graphAxes);
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
    bool isX = axis->orientation() == Qt::Horizontal;
    for (int i = 0; i < graphCount(); i++)
    {
        auto g = graph(i);

        if (!g->visible()) continue;
        
        if (g->property(PROP_GRAPH_SKIP_AUTOLIMITS).toBool())
            continue;

        if (isX) {
            if (g->keyAxis() != axis) continue;
        } else if (g->valueAxis() != axis) continue;

        bool hasRange = false;
        auto range = isX
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
    return axis->orientation() == Qt::Horizontal ? _safeMarginsX : _safeMarginsY;
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
    props.title = tr("Limits of %1").arg(axisIdent(axis));
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
    auto range = ((selectedAxis() == yAxis) ? yAxis : xAxis)->range();
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

bool Plot::axisFactorDlg(QCPAxis* axis)
{
    AxisFactorDlgProps props;
    props.title = tr("Factor of %1").arg(axisIdent(axis));
    props.plot = this;
    if (QCPL::axisFactorDlg(axis, props))
    {
        emit modified("Plot::axisFactorDlg");
        return true;
    }
    return false;
}

bool Plot::axisTextDlg(QCPAxis* axis)
{
    AxisFormatDlgProps props;
    props.title = tr("Title of %1").arg(axisIdent(axis));
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
    props.title = tr("Format of %1").arg(axisIdent(axis));
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
    props.title = tr("%1 Format").arg(axisIdent(scale->axis()));
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

QString Plot::axisTypeStr(QCPAxis::AxisType type) const {
    switch (type) {
    case QCPAxis::atLeft: return tr("Left Axis");
    case QCPAxis::atRight: return tr("Right Axis");
    case QCPAxis::atTop: return tr("Top Axis");
    case QCPAxis::atBottom: return tr("Bottom Axis");
    }
    return "Axis";
}

QString Plot::axisIdent(QCPAxis* axis) const
{
    if (axisIdents.contains(axis))
        return axisIdents[axis];
    auto type = axis->axisType();
    QString typeStr = axisTypeStr(type);
    if (axis == xAxis || axis == xAxis2 || axis == yAxis || axis == yAxis2)
        return typeStr;
    auto axes = axis->axisRect()->axes(type);
    for (int i = 0; i < axes.size(); i++)
        if (axes.at(i) == axis)
            return QString("%1 %2").arg(typeStr).arg(i);
    if (!axis->label().isEmpty())
        return axis->label();
    return tr("Axis");
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

bool Plot::addFormatter(void* target, TextFormatterBase* formatter)
{
    if (_formatters.contains(target)) {
        qWarning() << "Formatter is already registered for this target";
        return false;
    }
    _formatters[target] = formatter;
    return true;
}

bool Plot::ensureFormatter(void* target)
{
    if (_formatters.contains(target))
        return true;
    bool added = false;
    foreach (auto axis, axisRect()->axes()) {
        if (axis == target) {
            _formatters[target] = new AxisTextFormatter(axis);
            added = true;
            break;
        }
    }
    if (!added && target == _title) {
        _formatters[target] = new TitleTextFormatter(_title);
        added = true;
    }
    return added;
}

void Plot::putTextVar(void* target, const QString& name, const QString& descr, TextVarGetter getter)
{
    if (ensureFormatter(target))
        _formatters[target]->putVar(name, descr, getter);
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
    if (!_title->visible() || text.isEmpty())
    {
        // it's ok to get element without checking, but a console warning is printed then
        if (_backupLayout->hasElement(0, 0) && _backupLayout->element(0, 0) == _title)
            return;
        _backupLayout->addElement(0, 0, _title);
    }
    else
    {
        auto mainLayout = plotLayout();
        auto p = titleRC();
        if (mainLayout->hasElement(p.row, p.col) && mainLayout->element(p.row, p.col) == _title)
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
        if (!g->visible() && (flags & COUNT_ONLY_VISIBLE))
            continue;
        if (isService(g) && !(flags & COUNT_SERVICE))
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
            axis->setTicker(factorTicker->prevTicker);
    }
    else if (isAxisFactorSet(factor))
    {
        auto factorTicker = new FactorAxisTicker(axis->ticker());
        factorTicker->factor = factor;
        axis->setTicker(QSharedPointer<QCPAxisTicker>(factorTicker));
    }
    if (formatAxisTitleAfterFactorSet)
        updateText(axis);
    replot();
}

void Plot::initDefault(QCPAxis* axis)
{
    auto labelsFont = font();
#ifdef Q_OS_MAC
    labelsFont.setPointSize(14);
#else
    labelsFont.setPointSize(10);
#endif
    axis->setLabelFont(labelsFont);
    axis->setSelectedLabelFont(labelsFont);
    axis->setNumberPrecision(_numberPrecision);
    axis->setObjectName(QUuid::createUuid().toString(QUuid::Id128));
}

QCPAxis* Plot::addAxis(QCPAxis::AxisType axisType)
{
    if (axisType == QCPAxis::atBottom && xAxis && !xAxis->visible()) {
        xAxis->setVisible(true);
        return xAxis;
    }
    if (axisType == QCPAxis::atLeft && yAxis && !yAxis->visible()) {
        yAxis->setVisible(true);
        return yAxis;
    }
    if (axisType == QCPAxis::atTop && xAxis2 && !xAxis2->visible()) {
        xAxis2->setVisible(true);
        return xAxis2;
    }
    if (axisType == QCPAxis::atRight && yAxis2 && !yAxis2->visible()) {
        yAxis2->setVisible(true);
        return yAxis2;
    }
    auto axis = axisRect()->addAxis(axisType, new Axis(axisRect(), axisType));
    axis->setLayer(QLatin1String("axes"));
    axis->grid()->setLayer(QLatin1String("grid"));
    initDefault(axis);
    return axis;
}

QCPAxis* Plot::findAxisById(const QString &id)
{
    auto axes = axisRect()->axes();
    for (auto a : std::as_const(axes))
        if (a->objectName() == id)
            return a;
    return nullptr;
}

} // namespace QCPL
