// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_ui_graphics_PythonConsole_hpp
#define cf3_ui_graphics_PythonConsole_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QStringList>
#include <QQueue>
#include "ui/graphics/PythonCodeContainer.hpp"

#include "ui/graphics/LibGraphics.hpp"

class QToolBar;
class QStringListModel;
class QHBoxLayout;
class QTableWidget;
class QScrollArea;
class QTabWidget;
class QListWidget;
class QListWidgetItem;
class QWidget;
class QKeyEvent;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

class ListDebugValues;
class MainWindow;

/// @brief The python console send python command to the server in form of python "fragment"
/// the term "fragment" is used to describe a single python statement wich mean one instruction without tabulation (that instruction may have children instruction)
/// Example of python fragment:
/// for i in range(0,5):
///   print i
class Graphics_API PythonConsole : public PythonCodeContainer
{
  Q_OBJECT
public:
  /// @brief Construct a python console
  /// @param parent Parent widget.
  /// @param main_window Used to create a new script editor from the console
  PythonConsole(QWidget *parent,MainWindow* main_window);
  /// @brief called by the parent to create the splitter with the scope values
  void create_splitter(QTabWidget *tab_widget);
  /// @brief to check if the console can execute command (like for documentation request)
  bool is_stopped();
  /// @brief give acces to history_list_widget used in PythonCodeContainer to retrieve history from drag and drop
  const QListWidget* get_history_list_widget();
protected:
  void key_press_event(QKeyEvent *);
  void new_line(int indent_number);
  void border_click(const QPoint &pos);
  bool editable_zone(const QTextCursor &cursor);
  void mousePressEvent(QMouseEvent *e);
  void insert_text(const QString &text);
public slots:
  /// @brief parse and append the code to the console
  /// @param code The code to execute, can be a single command or a complete script
  /// @param immediate determine if the console must execute immediatly the command or let the user press enter after each commmands
  void execute_code(QString code,bool immediate,QVector<int> break_lines);
  /// @brief reduiced call, used in signal conversion
  void execute_code(QString code);
private slots:
  void insert_output(QString output, int fragment=-1);
  void insert_log(const QString &);
  void line_by_line_activated(bool);
  void stop_continue_pressed();
  void break_pressed();
  void execution_stopped(int fragment,int line);
  void stream_next_command();
  void push_history_to_script_editor();
  void scope_double_click(const QModelIndex & index);
  void history_double_click(const QModelIndex & index);
  void cursor_position_changed();
private:
  /// Index of the block that contains the current prompt
  int input_block;

  int input_start_in_text;

  void print_next_prompt();
  void push_input();
  void execute_input(QTextCursor &);
  void set_input(const QString &);
  void select_input(QTextCursor &);
  void fix_prompt();
  void add_history_draggable_item(const QString & text);

  class python_command{
  public:
    python_command(QString& command,bool imediate,QVector<int>& break_lines)
      : command(command)
      , imediate(imediate)
      , break_lines(break_lines) {}
    QString command;
    bool imediate;
    QVector<int> break_lines;
  };

  QAction* line_by_line;
  QAction* stop_continue;
  QAction* break_action;
  QStringList history;
  QString tmp_command;
  QIcon icon_stop,icon_continue;

  QQueue<python_command> command_stack;
  QVector<int> temporary_break_points;//store the break points of the input, in order to send them to the server before executing the command.
  QTimer auto_execution_timer;
  int history_index;
  bool stopped;

  bool text_being_entered;

  int block_count;

  MainWindow *main_window;
  QListWidget *history_list_widget;
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonConsole_hpp
