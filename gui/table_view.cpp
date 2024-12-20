#include <QToolTip>
#include <QTableView>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QMouseEvent>

#include "gui/table_view.h"

namespace leaf
{
task_table_view::task_table_view(QWidget *parent) : QTableView(parent)
{
    setShowGrid(false);
    horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter);
    setAlternatingRowColors(true);
    verticalHeader()->setVisible(false);
    setMouseTracking(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    horizontalHeader()->setMinimumWidth(60);
    verticalHeader()->setMinimumSectionSize(18);
    verticalHeader()->setMaximumSectionSize(30);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
void task_table_view::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
    {
        clearSelection();
    }
    else
    {
        QTableView::mousePressEvent(event);
    }
}
}    // namespace leaf
