#ifndef GRAPH_DATA_GRID_H
#define GRAPH_DATA_GRID_H

#include <QTableView>

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

class QCPGraph;

#include "qcpl_types.h"
#include "qcpl_export.h"

namespace QCPL {

class ValueFormatter;

class GraphDataGrid : public QTableView
{
    Q_OBJECT

public:
    explicit GraphDataGrid(QWidget *parent = nullptr);

    std::function<GraphDataExportSettings()> getExportSettings;

    /// Set a formatter for table cell values.
    /// Set `nullptr` to use default formatting.
    /// Table doesn't take ownership of the formatter.
    void setFormatter(const ValueFormatter *formatter);

    void setData(const ValueArray& x, const ValueArray& y);
    void setData(const GraphData& d) { setData(d.x, d.y); }
    void setData(QCPGraph* graph);

public slots:
    void copy();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QMenu *_contextMenu = nullptr;
};

} // namespace QCPL

#endif // GRAPH_DATA_GRID_H
