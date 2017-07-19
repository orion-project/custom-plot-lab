#ifndef GRAPH_DATA_GRID_H
#define GRAPH_DATA_GRID_H

#include <QTableWidget>

class GraphDataGrid : public QTableWidget
{
    Q_OBJECT

public:
    explicit GraphDataGrid();

    void setData(const QVector<double>& x, const QVector<double>& y);

public slots:
    void copy();

protected:
    void contextMenuEvent(QContextMenuEvent* event);

private:
    QMenu* _contextMenu = nullptr;

    QString formatValue(const double& value) const;
};

#endif // GRAPH_DATA_GRID_H
