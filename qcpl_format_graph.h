#ifndef QCPL_FORMAT_GRAPH_H
#define QCPL_FORMAT_GRAPH_H

#include <QWidget>

class QCPGraph;

namespace QCPL {

class PenEditorWidget;

class GraphFormatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphFormatWidget(QCPGraph *graph);

public slots:
    void apply();

private:
    QCPGraph *_graph;

    PenEditorWidget *_linePen;
};

} // namespace QCPL

#endif // QCPL_FORMAT_GRAPH_H
