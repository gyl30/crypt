#ifndef LEAF_GUI_FILES_WIDGET_H
#define LEAF_GUI_FILES_WIDGET_H

#include <QWidget>
#include <QAction>
#include <QTableView>
#include <QListWidget>
#include <QContextMenuEvent>

#include <map>
#include <vector>

#include "gui/gfile.h"
#include "file/event.h"

namespace leaf
{
class files_widget : public QWidget
{
    Q_OBJECT
   public:
    explicit files_widget(QWidget *parent);
    ~files_widget() override;

   public:
    void add_or_update_file(const leaf::gfile &file);

   public:
    void contextMenuEvent(QContextMenuEvent *event) override;

   Q_SIGNALS:
    void notify_event_signal(const leaf::notify_event &e);

   public:
    void on_new_file_clicked();
    void on_new_directory_clicked();

   private:
    QListWidget *list_widget_;
    std::string current_path_;
    QAction *new_file_action_;
    QAction *new_directory_action_;
    std::map<std::string, std::vector<gfile>> gfiles_;
};
}    // namespace leaf

#endif
