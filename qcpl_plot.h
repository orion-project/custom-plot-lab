#ifndef QCPL_PLOT_H
#define QCPL_PLOT_H

#include "qcustomplot/qcustomplot.h"

namespace QCPL {

typedef QCPGraph Graph;

class Plot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit Plot();

    QVector<Graph*>& serviceGraphs() { return _serviceGraphs; }

    bool graphAutoColors = true;
    bool autoReplot = true;

    Graph* makeNewGraph(const QString &title);
    Graph* makeNewGraph(const QString &title, const QVector<double>& x, const QVector<double>& y);

public slots:
    void autolimits();

signals:
    void graphSelected(Graph*);

protected:
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    
private slots:
    void plotSelectionChanged();
    //void graphClicked(QCPAbstractPlottable*,int,QMouseEvent*);

private:
    QVector<Graph*> _serviceGraphs;
    QCP::SelectionType _selectionType = QCP::stWhole;

    int _nextColorIndex = 0;

    bool isService(Graph* g) const { return _serviceGraphs.contains(g); }

    QColor nextGraphColor();

    QCPSelectionDecorator* makeSelectionDecorator(Graph* g) const;
};

} // namespace QCPL

#endif // QCPL_PLOT_H
