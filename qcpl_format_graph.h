#ifndef QCPL_FORMAT_GRAPH_H
#define QCPL_FORMAT_GRAPH_H

#include <QWidget>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QToolButton;
QT_END_NAMESPACE

namespace Ori {
namespace Widgets {
class MenuToolButton;
class BaseColorButton;
}}

class QCPGraph;

namespace QCPL {

class PenEditorWidget;
class ColorButton;

class GraphFormatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphFormatWidget(QCPGraph *graph);

    // Required for genericFormatDlg()
    bool needSaveDefault() const { return false; }

    void populate();

public slots:
    void apply();
    void restore();

private:
    QCPGraph *_graph;
    QJsonObject _backup;
    PenEditorWidget *_linePen, *_markerPen;
    QSpinBox *_markerSize, *_markerSkip;
    Ori::Widgets::MenuToolButton *_markerShape;
    Ori::Widgets::BaseColorButton *_markerColor;

    void createMarkerShapeAction(int shape, const QString& title);
    void selectMarkerColor();
    void setMarkerColor(const QColor& c);
};

} // namespace QCPL

#endif // QCPL_FORMAT_GRAPH_H
