#ifndef QCPL_FORMAT_GRAPH_H
#define QCPL_FORMAT_GRAPH_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QToolButton;
QT_END_NAMESPACE

namespace Ori {
namespace Widgets {
class MenuToolButton;
}}

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

    PenEditorWidget *_linePen, *_markerPen;
    QSpinBox *_markerSize, *_markerSkip;
    Ori::Widgets::MenuToolButton *_btnMarkerShape;
    QToolButton *_btnMarkerColor;
    QColor _markerColor;

    void createMarkerShapeAction(int shape, const QString& title);
    void selectMarkerColor();
    void setMarkerColor(const QColor& c);
};

} // namespace QCPL

#endif // QCPL_FORMAT_GRAPH_H
