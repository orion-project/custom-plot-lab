#include "GraphDataGrid.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMenu>

GraphDataGrid::GraphDataGrid() : QTableWidget()
{
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setColumnCount(2);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
#else
    horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
#endif
    setHorizontalHeaderLabels({ "X", "Y" });
}

void GraphDataGrid::setData(const QVector<double>& x, const QVector<double>& y)
{
    int count = qMin(x.size(), y.size());

    if (rowCount() != count)
        setRowCount(count);

    for (int i = 0; i < count; i++)
    {
        auto str_x = formatValue(x.at(i));
        auto str_y = formatValue(y.at(i));
        auto it = item(i, 0);
        if (!it)
        {
            it = new QTableWidgetItem(str_x);
            it->setTextAlignment(Qt::AlignRight);
            setItem(i, 0, it);

            it = new QTableWidgetItem(str_y);
            it->setTextAlignment(Qt::AlignRight);
            setItem(i, 1, it);

            resizeRowToContents(i);
        }
        else
        {
            it->setText(str_x);
            item(i, 1)->setText(str_y);
        }
    }
}

void GraphDataGrid::contextMenuEvent(QContextMenuEvent* event)
{
    QTableWidget::contextMenuEvent(event);

    if (!_contextMenu)
    {
        _contextMenu = new QMenu(this);
        _contextMenu->addAction(tr("Copy"), this, SLOT(copy()));
        _contextMenu->addAction(tr("Select All"), this, SLOT(selectAll()));
    }
    _contextMenu->popup(mapToGlobal(event->pos()));
}

// TODO copy entire selected range on Ctrl+C (currently only one cell is copied)
void GraphDataGrid::copy()
{
    QStringList x, y;
    foreach (QTableWidgetItem *it, selectedItems())
    {
        if (it->column() == 0)
            x.append(it->text());
        else
            y.append(it->text());
    }
    QString str;
    if (!x.isEmpty() && !y.isEmpty())
    {
        if (x.size() == y.size())
        {
            QStringList strs;
            for (int i = 0; i < x.size(); i++)
                strs.append(x.at(i) + '\t' + y.at(i));
            str = strs.join("\n");
        }
    }
    else if (!x.isEmpty())
        str = x.join("\n");
    else if (!y.isEmpty())
        str = y.join("\n");
    if (!str.isEmpty())
        qApp->clipboard()->setText(str);
}

QString GraphDataGrid::formatValue(const double& value) const
{
    return QString::number(value);
}
