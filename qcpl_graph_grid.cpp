#include "qcpl_graph_grid.h"

#include <QApplication>
#include <QAbstractItemModel>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QTableView>
#include <QTextStream>
#include <QMenu>

namespace {

class GraphDataModel : public QAbstractItemModel
{
public:
    GraphDataModel(QObject* parent): QAbstractItemModel(parent) {}
    QModelIndex index(int row, int col, const QModelIndex &parent) const override { Q_UNUSED(parent) return createIndex(row, col); }
    QModelIndex parent(const QModelIndex &child) const override { Q_UNUSED(child) return QModelIndex(); }
    int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent) return _x.size(); }
    int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent) return 2; }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role != Qt::DisplayRole) return QVariant();
        switch (orientation)
        {
        case Qt::Vertical:
            return section + 1;
        case Qt::Horizontal:
            return section == 0 ? QStringLiteral("X") : QStringLiteral("Y");
        }
        return QVariant();
    }
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role != Qt::DisplayRole) return QVariant();
        const QCPL::ValueArray& vals = index.column() == 0 ? _x : _y;
        // TODO: number formatter should be passed outside
        return QString::number(vals.at(index.row()), 'g', 10);
    }
    void setGraphData(const QCPL::ValueArray& x, const QCPL::ValueArray& y)
    {
        beginResetModel();
        _x = x;
        _y = y;
        endResetModel();
    }
private:
    QCPL::ValueArray _x, _y;
};

} // namespace

namespace QCPL {

GraphDataGrid::GraphDataGrid(QWidget *parent) : QTableView(parent)
{
    auto model = new GraphDataModel(this);

    setModel(model);
    setShowGrid(false);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    model->setGraphData({0.0}, {0.0});
    resizeRowsToContents();
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(rowHeight(0));
    model->setGraphData({}, {});
}

void GraphDataGrid::setData(const ValueArray& x, const ValueArray& y)
{
    dynamic_cast<GraphDataModel*>(model())->setGraphData(x, y);
}

void GraphDataGrid::contextMenuEvent(QContextMenuEvent* event)
{
    QTableView::contextMenuEvent(event);

    if (!_contextMenu)
    {
        _contextMenu = new QMenu(this);
        _contextMenu->addAction(tr("Copy"), this, SLOT(copy()));
        _contextMenu->addAction(tr("Select All"), this, SLOT(selectAll()));
    }
    _contextMenu->popup(mapToGlobal(event->pos()));
}

void GraphDataGrid::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy))
        copy();
    else
        QTableView::keyPressEvent(event);
}

void GraphDataGrid::copy()
{
    auto selection = selectionModel()->selection();
    if (selection.isEmpty()) return;
    QString text;
    QTextStream stream(&text);
    auto range = selection.first();
    int row1 = range.top();
    int row2 = range.bottom();
    int col1 = range.left();
    int col2 = range.right();
    bool twoCols = col1 != col2;
    for (int row = row1; row <= row2; row++)
    {
        stream << model()->data(model()->index(row, col1)).toString();
        if (twoCols)
            stream << '\t' << model()->data(model()->index(row, col2)).toString();
        if (row < row2)
            stream << '\n';
    }
    qApp->clipboard()->setText(text);
}

} // namespace QCPL
