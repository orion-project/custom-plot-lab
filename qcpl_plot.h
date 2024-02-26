#ifndef QCPL_PLOT_H
#define QCPL_PLOT_H

#include "qcpl_types.h"
#include "qcustomplot/qcustomplot.h"

namespace QCPL {

typedef QCPGraph Graph;

class TextFormatterBase;
class FormatSaver;

struct LayoutCell
{
    int row;
    int col;
};

class Plot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit Plot(QWidget* parent = nullptr);
    ~Plot();

    QVector<Graph*>& serviceGraphs() { return _serviceGraphs; }
    Graph* selectedGraph() const;

    enum GraphCountFlags {COUNT_DEFAULT = 0x0, COUNT_ONLY_VISIBLE = 0x1, COUNT_SERVICE = 0x2};
    int graphsCount(GraphCountFlags flags = COUNT_DEFAULT) const;

    bool graphAutoColors = true;
    bool useSafeMargins = true;
    bool excludeServiceGraphsFromAutolimiting = true;
    bool formatAxisTitleAfterFactorSet = false;

    /// Used for saving format settings of plot elements that can be used as 'default' setting.
    /// It is up to application when to load these stored default settings.
    QSharedPointer<FormatSaver> formatSaver;

    // Plot doesn't take ownership on context menus
    QMenu *menuAxisX = nullptr;
    QMenu *menuAxisY = nullptr;
    QMenu *menuGraph = nullptr;
    QMenu *menuPlot = nullptr;
    QMenu *menuLegend = nullptr;
    QMenu *menuTitle = nullptr;
    QMap<QCPLayerable*, QMenu*> menus;

    /// Display axis identifiers used for automatcally shown dialogs.
    /// E.g. plot shows limits dialog when an axis is double clicked,
    /// and the dialog should have some title. It tries to take a title from this map first
    /// and substitutes some default title if nothing has been found. @see getAxisIdent()
    QMap<QCPAxis*, QString> axisIdents;

    /// Map of objects with their store keys
    /// By default plot consists of axes, title, and legend.
    /// Additional parts (e.g. color scale of 2D graph) can be mentioned here
    /// in order not to loose them during copy/pasting or format saving.
    QHash<QCPLayerable*, QString> additionalParts;

    /// The callback used to get axis units for showing in the axis limits dialog.
    std::function<QString(QCPAxis*)> getAxisUnitString;

    Graph* makeNewGraph(const QString &title);
    Graph* makeNewGraph(const QString &title, const GraphData &data, bool replot = true);
    void updateGraph(Graph* graph, const GraphData &data, bool replot = true);

    /// Returns plot title. The plot has one predefined default title object.
    QCPTextElement* title() { return _title; }

    /// Puts the visible title into the main plot, and removes it from there if it's hidden.
    /// This is because QCustomPlot can't caclulate layout row height properly when an element is not visible.
    /// Even when the title is not visible, its height is accounted and layout row height ajusted accordingly.
    /// So we see an empty space above the plot instead of non visible title.
    /// This is undesired behaviour, so we need to remove the title from the main layout when it's hidden.
    void updateTitleVisibility();

    bool isFrameVisible() const;
    void setFrameVisible(bool on);

    AxisLimits limitsX() const { return limits(xAxis); }
    AxisLimits limitsY() const { return limits(yAxis); }
    AxisLimits limits(QCPAxis* axis) const;
    void setLimitsX(const AxisLimits& p, bool replot) { setLimitsX(p.min, p.max, replot); }
    void setLimitsY(const AxisLimits& p, bool replot) { setLimitsY(p.min, p.max, replot); }
    void setLimitsX(double min, double max, bool replot = true) { setLimits(xAxis, min, max, replot); }
    void setLimitsY(double min, double max, bool replot = true) { setLimits(yAxis, min, max, replot); }
    void setLimits(QCPAxis* axis, const AxisLimits& p, bool replot) { setLimits(axis, p.min, p.max, replot); }
    void setLimits(QCPAxis* axis, double min, double max, bool replot);
    void extendLimits(QCPAxis* axis, double factor, bool replot);
    void extendLimits(double factor, bool replot = true) { extendLimits(xAxis, factor, false); extendLimits(yAxis, factor, replot); }
    void extendLimitsX(double factor, bool replot = true) { extendLimits(xAxis, factor, replot); }
    void extendLimitsY(double factor, bool replot = true) { extendLimits(yAxis, factor, replot); }
    AxisFactor axisFactor(QCPAxis* axis) const;
    AxisFactor axisFactorX() const { return axisFactor(xAxis); }
    AxisFactor axisFactorY() const { return axisFactor(yAxis); }
    void setAxisFactorX(const AxisFactor& factor) { setAxisFactor(xAxis, factor); }
    void setAxisFactorY(const AxisFactor& factor) { setAxisFactor(yAxis, factor); }
    void setAxisFactor(QCPAxis* axis, const AxisFactor& factor);

    enum class PlotPart { None, AxisX, AxisY };
    PlotPart selectedPart() const;

    /// Initial layout row and column where the axis rect is placed.
    /// It's the row 1 because the row 0 is occupied by the plot title.
    /// Can be used for calculation of placement of additional elements, e.g. color scale of 2D plots.
    constexpr LayoutCell axisRectRC() const { return { 1, 0}; }

    /// Layout row and column when the title is placed.
    constexpr LayoutCell titleRC() const { return { 0, 0 }; }

    void addTextVar(void* target, const QString& name, const QString& descr, TextVarGetter getter);
    void addTextVarT(const QString& name, const QString& descr, TextVarGetter getter) { addTextVar(_title, name, descr, getter); }
    void addTextVarX(const QString& name, const QString& descr, TextVarGetter getter) { addTextVar(xAxis, name, descr, getter); }
    void addTextVarY(const QString& name, const QString& descr, TextVarGetter getter) { addTextVar(yAxis, name, descr, getter); }

    void addFormatter(void* target, TextFormatterBase* formatter);
    TextFormatterBase* formatter(void* target) const { return _formatters.contains(target) ? _formatters[target] : nullptr; }
    void setFormatterText(void* target, const QString& text);
    void setFormatterTextT(const QString& text) { setFormatterText(_title, text); }
    void setFormatterTextX(const QString& text) { setFormatterText(xAxis, text); }
    void setFormatterTextY(const QString& text) { setFormatterText(yAxis, text); }
    QString formatterText(void* target) const;
    QString formatterTextT() const { return formatterText(_title); }
    QString formatterTextX() const { return formatterText(xAxis); }
    QString formatterTextY() const { return formatterText(yAxis); }

    void updateTexts();
    void updateText(void* target);
    void updateTextT() { updateText(_title); }
    void updateTextX() { updateText(yAxis); }
    void updateTextY() { updateText(xAxis); }

    QString defaultText(void* target) const { return _defaultTexts.contains(target) ? _defaultTexts[target] : QString(); }
    QString defaultTextT() const { return defaultText(_title); }
    QString defaultTextX() const { return defaultText(xAxis); }
    QString defaultTextY() const { return defaultText(yAxis); }
    void setDefaultText(void* target, const QString& text) { _defaultTexts[target] = text; }
    void setDefaultTextT(const QString& text) { setDefaultText(_title, text); }
    void setDefaultTextX(const QString& text) { setDefaultText(xAxis, text); }
    void setDefaultTextY(const QString& text) { setDefaultText(yAxis, text); }

    bool limitsDlg(QCPAxis* axis);
    bool axisFactorDlg(QCPAxis* axis);
    bool axisTextDlg(QCPAxis* axis);
    bool axisFormatDlg(QCPAxis* axis);
    bool colorScaleFormatDlg(QCPColorScale* axis);
    void autolimits(QCPAxis* axis, bool replot);

public slots:
    void autolimits(bool replot = true) { autolimits(xAxis, false); autolimits(yAxis, replot); }
    void autolimitsX(bool replot = true) { autolimits(xAxis, replot); }
    void autolimitsY(bool replot = true) { autolimits(yAxis, replot); }
    bool limitsDlgX() { return limitsDlg(xAxis); }
    bool limitsDlgY() { return limitsDlg(yAxis); }
    bool limitsDlgXY();
    bool axisFactorDlgX() { return axisFactorDlg(xAxis); }
    bool axisFactorDlgY() { return axisFactorDlg(yAxis); }
    void zoomIn() { extendLimits(-(_zoomStepX+_zoomStepY)/2.0); }
    void zoomOut() { extendLimits((_zoomStepX+_zoomStepY)/2.0); }
    void zoomInX() { extendLimitsX(-_zoomStepX); }
    void zoomOutX() { extendLimitsX(_zoomStepX); }
    void zoomInY() { extendLimitsY(-_zoomStepY); }
    void zoomOutY() { extendLimitsY(_zoomStepY); }
    void copyPlotImage();
    bool axisTextDlgX() { return axisTextDlg(xAxis); }
    bool axisTextDlgY() { return axisTextDlg(yAxis); }
    bool axisFormatDlgX() { return axisFormatDlg(xAxis); }
    bool axisFormatDlgY() { return axisFormatDlg(yAxis); }
    bool titleTextDlg();
    bool titleFormatDlg();
    bool legendFormatDlg();

signals:
    void graphClicked(Graph*);
    void emptySpaceDoubleClicked(QMouseEvent *event);
    void resized(const QSize& oldSize, const QSize& newSize);
    void modified(const QString& reason);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    
private slots:
    void plotSelectionChanged();
    void rawGraphClicked(QCPAbstractPlottable*);
    void axisDoubleClicked(QCPAxis*, QCPAxis::SelectablePart);

private:
    QVector<Graph*> _serviceGraphs;
    QCP::SelectionType _selectionType = QCP::stWhole;
    QCPTextElement *_title;
    int _nextColorIndex = 0;
    const double _safeMarginsX;
    const double _safeMarginsY;
    const double _zoomStepX;
    const double _zoomStepY;
    const int _numberPrecision;
    QMap<void*, TextFormatterBase*> _formatters;
    QMap<void*, QString> _defaultTexts;
    QCPLayoutGrid *_backupLayout;

    bool isService(Graph* g) const { return _serviceGraphs.contains(g); }

    QColor nextGraphColor();

    void setAxisRange(QCPAxis* axis, const QCPRange &range);
    double safeMargins(QCPAxis* axis);
    QString getAxisIdent(QCPAxis* axis) const;
};


} // namespace QCPL

#endif // QCPL_PLOT_H
