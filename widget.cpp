#include "widget.h"
#include <qnamespace.h>
#include <QAbstractTableModel>
#include <QApplication>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QList>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyleOptionProgressBar>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTimer>
#include <QEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QToolTip>
#include <QVBoxLayout>
#include <random>
#include <vector>
#include <string>

struct task
{
    std::string src_file;
    std::string dst_file;
    std::string src_hash;
    std::string dst_hash;
    int progress = 0;
};
std::string random_string(int len)
{
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    std::random_device rd;
    std::mt19937 generator(rd());

    std::shuffle(str.begin(), str.end(), generator);

    return str.substr(0, len);
}

class task_style_delegate : public QStyledItemDelegate
{
   public:
    explicit task_style_delegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

   public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (!index.isValid())
        {
            return QStyledItemDelegate::paint(painter, option, index);
        }
        if (index.column() != 4)
        {
            return QStyledItemDelegate::paint(painter, option, index);
        }

        painter->save();

        const QStyle *style(QApplication::style());

        QStyleOptionProgressBar p;
        p.state = option.state | QStyle::State_Horizontal | QStyle::State_Small;
        p.direction = QApplication::layoutDirection();
        p.rect = option.rect;
        p.rect.moveCenter(option.rect.center());
        p.minimum = 0;
        p.maximum = 100;
        p.textAlignment = Qt::AlignCenter;
        p.textVisible = true;
        p.progress = index.model()->data(index, Qt::DisplayRole).toInt();
        p.text = QStringLiteral("处理中 %1%").arg(p.progress);
        if (p.progress >= 100)
        {
            p.text = QString("处理完成");
        }
        style->drawControl(QStyle::CE_ProgressBar, &p, painter);
        painter->restore();
    }
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const override { return QSize(option.rect.size()); }
};

class task_model : public QAbstractTableModel
{
   public:
    explicit task_model(QObject *parent = nullptr) : QAbstractTableModel(parent)
    {
        init_tasks();
        start_timer();
    }

   public:
    [[nodiscard]] int rowCount(const QModelIndex & /*parent*/) const override { return static_cast<int>(tasks_.size()); }

    [[nodiscard]] int columnCount(const QModelIndex & /*parent*/) const override { return 5; }

    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override
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
    [[nodiscard]] static QVariant set_header_data(int section, int role)
    {
        if (role == Qt::DisplayRole)
        {
            QString value;
            switch (section)
            {
                case 0:
                    value = "源文件";
                    break;
                case 1:
                    value = "源文件 HASH";
                    break;
                case 2:
                    value = "目标文件";
                    break;
                case 3:
                    value = "目标文件 HASH";
                    break;
                case 4:
                    value = "处理进度";
                    break;
                default:
                    break;
            }
            return value;
        }
        return {};
    }
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override
    {
        if (role != Qt::DisplayRole)
        {
            return {};
        }
        return task_to_display_data(index);
    }

    [[nodiscard]] QVariant task_to_display_data(const QModelIndex &index) const
    {
        if (!index.isValid() || index.column() >= columnCount(index) || index.row() >= rowCount(index))
        {
            return {};
        }
        const int row = index.row();
        const auto &t = tasks_[row];
        const int column = index.column();
        if (column == 0)
        {
            return QString::fromStdString(t.src_file);
        }
        if (column == 1)
        {
            return QString::fromStdString(t.src_hash);
        }
        if (column == 2)
        {
            return QString::fromStdString(t.dst_file);
        }
        if (column == 3)
        {
            return QString::fromStdString(t.dst_hash);
        }
        if (column == 4)
        {
            return t.progress;
        }
        return {};
    }

   private:
    void start_timer()
    {
        timer_ = new QTimer(this);
        auto c = connect(timer_, &QTimer::timeout, this, &task_model::on_timer);
        (void)c;
        timer_->start(1000);
    }
    void init_tasks()
    {
        for (int i = 0; i < 5; i++)
        {
            task t;
            t.src_file = random_string(8);
            t.dst_file = random_string(12);
            t.src_hash = random_string(64);
            t.dst_hash = random_string(64);
            t.progress = 0;
            tasks_.push_back(t);
        }
    }
    void on_timer()
    {
        if (tasks_.empty())
        {
            return;
        }
        beginResetModel();
        for (auto &t : tasks_)
        {
            t.progress += 1;
        }
        endResetModel();
    }

   private:
    std::vector<task> tasks_;
    QTimer *timer_ = nullptr;
};

class task_table_view : public QTableView
{
   public:
    explicit task_table_view(QWidget *parent) : QTableView(parent)
    {
        setShowGrid(false);
        horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter);
        setAlternatingRowColors(true);
        verticalHeader()->setVisible(false);
        horizontalHeader()->setStretchLastSection(true);
        setMouseTracking(true);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        horizontalHeader()->setMinimumWidth(60);
        verticalHeader()->setMinimumSectionSize(18);
        verticalHeader()->setMaximumSectionSize(30);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        auto c = connect(this, &QTableView::entered, this, &task_table_view::show_tooltip);
        (void)c;
    }

   public:
    static void show_tooltip(const QModelIndex &index)
    {
        if (!index.isValid())
        {
            return;
        }
        if (index.column() == 4)
        {
            QToolTip::showText(QCursor::pos(), QStringLiteral("处理中 %1%").arg(index.data().toInt()));
            return;
        }

        QToolTip::showText(QCursor::pos(), index.data().toString());
    }
};

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    list_view_ = new task_table_view(this);

    auto *model = new task_model();
    list_view_->setModel(model);

    auto *delegate = new task_style_delegate();
    list_view_->setItemDelegate(delegate);

    auto *new_file_btn = new QPushButton();
    new_file_btn->setText("Add New File");

    auto *layout = new QHBoxLayout();
    layout->addWidget(new_file_btn);
    layout->addWidget(list_view_);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    resize(800, 300);
}

Widget::~Widget() = default;