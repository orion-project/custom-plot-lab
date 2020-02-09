#ifndef QCPL_PLOT_H
#define QCPL_PLOT_H

#include "qcustomplot/qcustomplot.h"
#include "qcpl_types.h"

namespace QCPL {

typedef QCPGraph Graph;

class Plot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit Plot(QWidget* parent = nullptr);

    QVector<Graph*>& serviceGraphs() { return _serviceGraphs; }

    bool graphAutoColors = true;
    bool useSafeMargins = true;
    bool excludeServiceGraphsFromAutolimiting = true;

    QMenu *menuAxisX = nullptr;
    QMenu *menuAxisY = nullptr;

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

public slots:
    void autolimits(bool replot = true) { autolimits(xAxis, false); autolimits(yAxis, replot); }
    void autolimitsX(bool replot = true) { autolimits(xAxis, replot); }
    void autolimitsY(bool replot = true) { autolimits(yAxis, replot); }
    bool setLimitsDlgX() { return setLimitsDlg(xAxis); }
    bool setLimitsDlgY() { return setLimitsDlg(yAxis); }
    void zoomIn() { extendLimits(-(_zoomStepX+_zoomStepY)/2.0); }
    void zoomOut() { extendLimits((_zoomStepX+_zoomStepY)/2.0); }
    void zoomInX() { extendLimitsX(-_zoomStepX); }
    void zoomOutX() { extendLimitsX(_zoomStepX); }
    void zoomInY() { extendLimitsY(-_zoomStepY); }
    void zoomOutY() { extendLimitsY(_zoomStepY); }

signals:
    void graphSelected(Graph*);
    void emptySpaceDoubleClicked(QMouseEvent *event);
    void editTitleRequest();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    
private slots:
    void plotSelectionChanged();
    void graphClicked(QCPAbstractPlottable*);
    bool setLimitsDlg(QCPAxis* axis);

private:
    enum class PlotPart { None, AxisX, AxisY };

    QVector<Graph*> _serviceGraphs;
    QCP::SelectionType _selectionType = QCP::stWhole;
    QCPTextElement *_title = nullptr;
    int _nextColorIndex = 0;
    const double _safeMarginsX;
    const double _safeMarginsY;
    const double _zoomStepX;
    const double _zoomStepY;
    const int _numberPrecision;

    bool isService(Graph* g) const { return _serviceGraphs.contains(g); }

    QColor nextGraphColor();

    void autolimits(QCPAxis* axis, bool replot);
    void extendLimits(QCPAxis* axis, double factor, bool replot);
    bool setLimitsDlg(QCPRange& range, const QString &title, const QString& unit);
    void setAxisRange(QCPAxis* axis, const QCPRange &range);
    double safeMargins(QCPAxis* axis);
    PlotPart selectedPart() const;
    QString getAxisTitle(QCPAxis* axis) const;
};

} // namespace QCPL

#endif // QCPL_PLOT_H
