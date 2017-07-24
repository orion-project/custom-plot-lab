#ifndef GRAPH_DATA_GRID_H
#define GRAPH_DATA_GRID_H

#include <QTableWidget>

#include "qcpl_types.h"

namespace QCPL {

class GraphDataGrid : public QTableWidget
{
    Q_OBJECT

public:
    explicit GraphDataGrid();

    void setData(const ValueArray& x, const ValueArray& y);
    void setData(const GraphData& d) { setData(d.x, d.y); }

public slots:
    void copy();

protected:
    void contextMenuEvent(QContextMenuEvent* event);

private:
    QMenu* _contextMenu = nullptr;

    QString formatValue(const double& value) const;
};

} // namespace QCPL

#endif // GRAPH_DATA_GRID_H
