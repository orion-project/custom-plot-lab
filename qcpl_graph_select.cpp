#include "qcpl_graph_select.h"

#include "qcpl_plot.h"

#include <helpers/OriDialogs.h>
#include <helpers/OriLayouts.h>
#include <helpers/OriWidgets.h>

#include <QTableWidget>

namespace QCPL {

QToolButton* makeButton(QAction *action)
{
    auto b = new QToolButton;
    b->setToolTip(QString("<p>%1<p><b>%2</b>").arg(action->text(), action->shortcut().toString()));
    b->setIcon(action->icon());
    b->connect(b, &QToolButton::clicked, action, &QAction::trigger);
    // TODO: adjust to dark theme or link to global app styles somehow
    b->setStyleSheet(QStringLiteral(
        "QToolButton{border: 1px solid transparent; border-radius: 3px; padding: 1px}"
        "QToolButton:hover{border-color: silver; background-color:"
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fefefe, stop:1 #f6f7f9)}"
    ));
    return b;
}

bool selectGraphsDlg(Plot *plot)
{
    QSharedPointer<QTableWidget> table(new QTableWidget);
    table->setWordWrap(false);
    table->setColumnCount(2);
    table->setShowGrid(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    table->horizontalHeader()->resizeSection(1, 40);
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->connect(table.data(), &QTableWidget::cellDoubleClicked, [table](int row, int col){
        bool checked = table->item(row, 0)->checkState() == Qt::Checked;
        table->item(row, 0)->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
    });
    
    auto selectAll = [table]{
        for (int i = 0; i < table->rowCount(); i++)
            table->item(i, 0)->setCheckState(Qt::Checked);
    };
    auto selectNone = [table]{
        for (int i = 0; i < table->rowCount(); i++)
            table->item(i, 0)->setCheckState(Qt::Unchecked);
    };
    auto selectInvert = [table]{
        for (int i = 0; i < table->rowCount(); i++) {
            auto item = table->item(i, 0);
            bool checked = item->checkState() == Qt::Checked;
            item->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
        }
    };
    
    auto actnSelectAll = Ori::Gui::action(qApp->tr("Select All"), table.data(), selectAll, ":/qcpl_images/check_all", QKeySequence("Ctrl+A"));
    auto actnSelectNone = Ori::Gui::action(qApp->tr("Select None"), table.data(), selectNone, ":/qcpl_images/check_none", QKeySequence("Ctrl+D"));
    auto actnSelectInvert = Ori::Gui::action(qApp->tr("Invert Selection"), table.data(), selectInvert, ":/qcpl_images/check_invert", QKeySequence("Ctrl+I"));
    table->addAction(actnSelectAll);
    table->addAction(actnSelectNone);
    table->addAction(actnSelectInvert);

    QList<QCPGraph*> graphs;
    for (int i = 0; i < plot->graphCount(); i++)
    {
        auto g = plot->graph(i);
        if (!g->visible()) continue;
        if (g->selectable() == QCP::stNone) continue;
        graphs << g;
    }

    table->setRowCount(graphs.size());
    for (int row = 0; row < graphs.size(); row++)
    {
        auto g = graphs.at(row);
        
        auto it = new QTableWidgetItem;
        it->setText(g->name());
        it->setCheckState(g->selected() ? Qt::Checked : Qt::Unchecked);
        it->setData(Qt::UserRole, QVariant::fromValue(g));
        table->setItem(row, 0, it);
        
        auto it1 = new QTableWidgetItem;
        it1->setBackground(g->pen().brush());
        it1->setFlags(Qt::ItemIsEnabled);
        table->setItem(row, 1, it1);
        
        table->setRowHeight(row, 22);
    }
    table->setCurrentCell(0, 0);
    
    Ori::Dlg::Dialog dlg(table);
    bool ok = dlg
        .withTitle(qApp->tr("Select Graphs"))
        .withStretchedContent()
        .withPersistenceId("qpl_select_graphs_dlg")
        .withOnDlgShown([&dlg, actnSelectAll, actnSelectNone, actnSelectInvert]{
            auto buttonBoxLayout = dynamic_cast<QBoxLayout*>(dlg.okButton()->parentWidget()->layout());
            if (buttonBoxLayout)
            {
                buttonBoxLayout->insertLayout(0, Ori::Layouts::LayoutH({
                    makeButton(actnSelectAll),
                    makeButton(actnSelectNone),
                    makeButton(actnSelectInvert),
                    Ori::Layouts::Space(24)
                }).setMargin(0).setSpacing(0).boxLayout());
            }
        })
        .exec();
    if (ok)
    {
        plot->deselectAll();
        for (int row = 0; row < table->rowCount(); row++)
        {
            auto item = table->item(row, 0);
            if (item->checkState() == Qt::Checked)
                plot->selectGraph(item->data(Qt::UserRole).value<QCPGraph*>());
        }
        plot->updateAxesInteractivity();
        plot->replot();
    }
    return ok;
}

} // namespace QCPL
