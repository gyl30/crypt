#include "gui/table_model.h"

namespace leaf
{
task_model::task_model(QObject *parent) : QAbstractTableModel(parent) {}

void task_model::add_or_update_task(const leaf::task &task)
{
    bool update = false;
    beginResetModel();
    for (auto &&t : tasks_)
    {
        if (task.op == t.op && task.filename == t.filename)
        {
            t.process_size = task.process_size;
            t.file_size = task.file_size;
            update = true;
            break;
        }
    }
    if (!update)
    {
        tasks_.push_back(task);
    }

    endResetModel();
}

void task_model::delete_task(const leaf::task &task)
{
    beginResetModel();
    tasks_.erase(
        std::remove_if(tasks_.begin(),
                       tasks_.end(),
                       [&task](const leaf::task &t) { return t.op == task.op && t.filename == task.filename; }),
        tasks_.end());
    endResetModel();
}
int task_model::rowCount(const QModelIndex & /*parent*/) const { return static_cast<int>(tasks_.size()); }

int task_model::columnCount(const QModelIndex & /*parent*/) const { return 2; }

QVariant task_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return set_header_data(section, role);
    }
    if (orientation == Qt::Vertical)
    {
        return {};
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}
QVariant task_model::set_header_data(int section, int role)
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }
    if (section == 0)
    {
        return "源文件";
    }
    if (section == 1)
    {
        return "处理进度";
    }
    return {};
}

QVariant task_model::tooltip_data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() >= columnCount(index) || index.row() >= rowCount(index))
    {
        return {};
    }
    const size_t row = index.row();
    if (row >= tasks_.size())
    {
        return {};
    }
    const auto &t = tasks_[row];
    const int column = index.column();
    if (role == Qt::ToolTipRole && column == 1)
    {
        if (t.process_size == 0)
        {
            return "等待中";
        }
        return "处理中";
    }
    return {};
}

QVariant task_model::display_data(const QModelIndex &index, int /*role*/) const
{
    if (!index.isValid() || index.column() >= columnCount(index) || index.row() >= rowCount(index))
    {
        return {};
    }
    const size_t row = index.row();
    if (row >= tasks_.size())
    {
        return {};
    }
    const auto &t = tasks_[row];
    const int column = index.column();
    if (column == 0)
    {
        return QString::fromStdString(t.filename);
    }
    if (column == 1 && t.process_size != 0)
    {
        return static_cast<double>(t.process_size) * 100 / static_cast<double>(t.file_size);
    }
    if (column == 1 && t.process_size == 0)
    {
        return 0;
    }
    return {};
}

QVariant task_model::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return display_data(index, role);
    }
    if (role == Qt::ToolTipRole)
    {
        return tooltip_data(index, role);
    }
    return {};
}

}    // namespace leaf
