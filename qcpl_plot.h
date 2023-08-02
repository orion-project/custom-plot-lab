#ifndef QCPL_PLOT_H
#define QCPL_PLOT_H

#include "qcpl_types.h"
#include "qcustomplot/qcustomplot.h"

namespace QCPL {

typedef QCPGraph Graph;

class TextFormatterBase;
class FormatSaver;

class Plot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit Plot(QWidget* parent = nullptr);
    ~Plot();

    QVector<Graph*>& serviceGraphs() { return _serviceGraphs; }
    Graph* selectedGraph() const;

    bool graphAutoColors = true;
    bool useSafeMargins = true;
    bool excludeServiceGraphsFromAutolimiting = true;
    QSharedPointer<FormatSaver> formatSaver;

    // Plot doesn't take ownership on context menus
    QMenu *menuAxisX = nullptr;
    QMenu *menuAxisY = nullptr;
    QMenu *menuGraph = nullptr;
    QMenu *menuPlot = nullptr;
    QMenu *menuLegend = nullptr;

    /// The callback used to get axis units for showing in the axis limits dialog.
    std::function<QString(QCPAxis*)> getAxisUnitString;

    Graph* makeNewGraph(const QString &title);
    Graph* makeNewGraph(const QString &title, const GraphData &data, bool replot = true);
    void updateGraph(Graph* graph, const GraphData &data, bool replot = true);

    QCPTextElement* title() { return _title; }
    void setTitleVisible(bool on);

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
    void extendLimits(double factor, bool replot = true);
    void extendLimitsX(double factor, bool replot = true) { extendLimits(xAxis, factor, replot); }
    void extendLimitsY(double factor, bool replot = true) { extendLimits(yAxis, factor, replot); }

    enum class PlotPart { None, AxisX, AxisY };
    PlotPart selectedPart() const;

    // TODO: should be a row below the title, it can be 0 or 1, depending on if title is visible
    int axisRectRow() const { return 0; }
    int axisRectCol() const { return 0; }

    void addTextVar(void* target, const QString& name, const QString& descr, TextVarGetter getter);
    void addTextVar(const QString& name, const QString& descr, TextVarGetter getter) { addTextVar(_title, name, descr, getter); }
    void addTextVarX(const QString& name, const QString& descr, TextVarGetter getter) { addTextVar(xAxis, name, descr, getter); }
    void addTextVarY(const QString& name, const QString& descr, TextVarGetter getter) { addTextVar(yAxis, name, descr, getter); }

    void addFormatter(void* target, TextFormatterBase* formatter);
    TextFormatterBase* formatter(void* target) const { return _formatters.contains(target) ? _formatters[target] : nullptr; }
    void setFormatterText(void* target, const QString& text);
    void setFormatterText(const QString& text) { setFormatterText(_title, text); }
    void setFormatterTextX(const QString& text) { setFormatterText(xAxis, text); }
    void setFormatterTextY(const QString& text) { setFormatterText(yAxis, text); }
    QString formatterText(void* target) const;
    QString formatterText() const { return formatterText(_title); }
    QString formatterTextX() const { return formatterText(xAxis); }
    QString formatterTextY() const { return formatterText(yAxis); }

    void updateTitles();
    void updateTitle(void* target);
    void updateTitle() { updateTitle(_title); }
    void updateTitleX() { updateTitle(yAxis); }
    void updateTitleY() { updateTitle(xAxis); }

    QString defaultTitle(void* target) const { return _defaultTitles.contains(target) ? _defaultTitles[target] : QString(); }
    QString defaultTitle() const { return defaultTitle(_title); }
    QString defaultTitleX() const { return defaultTitle(xAxis); }
    QString defaultTitleY() const { return defaultTitle(yAxis); }
    void setDefaultTitle(void* target, const QString& text) { _defaultTitles[target] = text; }
    void setDefaultTitle(const QString& text) { setDefaultTitle(_title, text); }
    void setDefaultTitleX(const QString& text) { setDefaultTitle(xAxis, text); }
    void setDefaultTitleY(const QString& text) { setDefaultTitle(yAxis, text); }

public slots:
    void autolimits(bool replot = true) { autolimits(xAxis, false); autolimits(yAxis, replot); }
    void autolimitsX(bool replot = true) { autolimits(xAxis, replot); }
    void autolimitsY(bool replot = true) { autolimits(yAxis, replot); }
    bool limitsDlgX() { return limitsDlg(xAxis); }
    bool limitsDlgY() { return limitsDlg(yAxis); }
    bool limitsDlgXY();
    void zoomIn() { extendLimits(-(_zoomStepX+_zoomStepY)/2.0); }
    void zoomOut() { extendLimits((_zoomStepX+_zoomStepY)/2.0); }
    void zoomInX() { extendLimitsX(-_zoomStepX); }
    void zoomOutX() { extendLimitsX(_zoomStepX); }
    void zoomInY() { extendLimitsY(-_zoomStepY); }
    void zoomOutY() { extendLimitsY(_zoomStepY); }
    void copyPlotImage();
    bool titleDlgX() { return titleDlg(xAxis); }
    bool titleDlgY() { return titleDlg(yAxis); }
    bool formatDlgX() { return formatDlg(xAxis); }
    bool formatDlgY() { return formatDlg(yAxis); }
    bool formatDlg0();
    bool formatDlgLegend();

signals:
    void graphClicked(Graph*);
    void emptySpaceDoubleClicked(QMouseEvent *event);
    void editTitleRequest();
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
    QCPTextElement *_title = nullptr;
    int _nextColorIndex = 0;
    const double _safeMarginsX;
    const double _safeMarginsY;
    const double _zoomStepX;
    const double _zoomStepY;
    const int _numberPrecision;
    QMap<void*, TextFormatterBase*> _formatters;
    QMap<void*, QString> _defaultTitles;
    QMap<QString, QVariant> _backup;

    bool isService(Graph* g) const { return _serviceGraphs.contains(g); }

    QColor nextGraphColor();

    bool limitsDlg(QCPAxis* axis);
    bool titleDlg(QCPAxis* axis);
    bool formatDlg(QCPAxis* axis);
    void autolimits(QCPAxis* axis, bool replot);
    void extendLimits(QCPAxis* axis, double factor, bool replot);
    void setAxisRange(QCPAxis* axis, const QCPRange &range);
    double safeMargins(QCPAxis* axis);
    QString getAxisIdent(QCPAxis* axis) const;
};


} // namespace QCPL

#endif // QCPL_PLOT_H
