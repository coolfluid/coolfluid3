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
#include <QPainter>
#include <QListWidget>
#include "ui/graphics/PythonCodeContainer.hpp"

#include "ui/graphics/LibGraphics.hpp"
#include <iostream>

class QToolBar;
class QStringListModel;
class QHBoxLayout;
class QTableWidget;
class QScrollArea;
class QTabWidget;
class QListWidgetItem;
class QWidget;
class QKeyEvent;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

class ListDebugValues;
class MainWindow;
class CustomListWidget;

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
  void create_python_area(QWidget *widget);
  /// @brief to check if the console can execute command (like for documentation request)
  bool is_stopped();
  /// @brief give acces to history_list_widget used in PythonCodeContainer to retrieve history from drag and drop
  const QListWidget* get_history_list_widget();
  /// @brief return the block number where the debug arrow is, -1 if the execution is not breaked
  int get_debug_arrow_block();
  /// @brief ask to the script engine to toggle the break point on the given fragment at the given line number
  void send_toggle_break_point(int fragment_block,int line_number);
protected:
  /// @brief called by PythonCodeContainer when the key event is not managed by him
  void key_press_event(QKeyEvent *);
  /// @brief called at each new line
  void new_line(int indent_number);
  /// @brief called when the user click on the border area to toggle a break point
  void border_click(const QPoint &pos);
  /// @brief called by PythonCodeContainer to know if the zone is editable
  bool editable_zone(const QTextCursor &cursor);
  /// @brief called by PythonCodeContainer when a text need to be inserted
  void insert_text(const QString &text);
public slots:
  /// @brief parse and append the code to the console
  /// @param code The code to execute, can be a single command or a complete script
  /// @param immediate determine if the console must execute immediatly the command or let the user press enter after each commmands
  void execute_code(QString code,bool immediate,QVector<int> break_lines);
  /// @brief reduiced call, used in signal conversion
  void execute_code(QString code);
private slots:
  /// @brief called by NScriptEngine when the execution is breaked, the debug arrow will be showed at
  void display_debug_trace(int fragment,int line);
  /// @brief change the fragment index with the new one, used in multi-client communication
  void change_code_fragment(int fragment,int new_fragment);
  /// @brief clear the debug arrow
  void reset_debug_trace();
  /// @brief insert output for a particular fragment
  void insert_output(QString output, int fragment=-1);
  /// @brief called by the log manager, the log will be shown at the last fragment
  void insert_log(const QString &);
  /// @brief active the line by line execution, called by the toolbar
  void line_by_line_activated(bool);
  /// @brief if the script engine is executing some this will ask to stop the execution
  /// if the script engine where breaked this will continue the execution
  void stop_continue_pressed();
  /// @brief if the script engine is stopped this will ask to go out of the current fragment, usefull for infinite loop by example
  void break_pressed();
  /// @brief
  void execution_stopped(int fragment,int line);
  /// @brief if they are some commands on the command stack this function will execute then one by one
  void stream_next_command();
  /// @brief append all the history on a new PythonScriptEditor
  void push_history_to_script_editor();
  /// @brief when the user double click on the scope this will append the symbole to the PythonConsole
  void scope_double_click(const QModelIndex & index);
  /// @brief when the user double click on the history this will append the command to the PythonConsole
  void history_double_click(const QModelIndex & index);
  /// @brief
  void cursor_position_changed();
  /// @brief append a command on the PythonConsole as if this command where executed but it is just for display purpose
  /// , the code is not actually executed
  void append_false_code(QString code);
private:
  /// @brief used to generate the fragment index
  int fragment_generator;
  /// @brief store the fragment number for each block
  QMap<int,int> blocks_fragment;
  /// @brief store the block number for each fragment
  QMap<int,int> fragment_container;
  /// @brief Index of the block that contains the current prompt
  int input_block;
  /// @brief position in character where the last prompt start
  int input_start_in_text;
  /// @brief block number of the debug arrow, -1 for no arrow
  int debug_arrow;
  /// @brief display the next prompt after executing the last one
  void print_next_prompt();
  /// @brief
  //void push_input(); ???
  /// @brief execute the current statement
  void execute_input(QTextCursor &);
  /// @brief replace the current prompted code with the given text
  void set_input(const QString &);
  /// @brief select the current prompted code with the on the given text cursor
  void select_input(QTextCursor &);
  /// @brief correct the block state on the current prompt
  void fix_prompt();
  /// @brief add to the history a new entry
  void add_history_draggable_item(const QString & text);
  /// @brief Send the python code fragment to the script engine
  void register_fragment(QString code,int block_number,QVector<int> break_point);

  /// @brief used to store the pending command on the command stack
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
  /// @brief toolbar line by line action
  QAction* line_by_line;
  /// @brief toolbar stop continue action
  QAction* stop_continue;
  /// @brief toolbar break action
  QAction* break_action;
  /// @brief store the history, used when up key and down key are pressed
  QStringList history;
  /// @brief store the current history index when using the up and down keys
  int history_index;
  /// @brief store a temporary command when some code need to be executed and they where some code on the current prompt
  QString tmp_command;
  /// @brief used to change the icon on the stop_continue action
  QIcon icon_stop,icon_continue;
  /// @brief store the pending commands
  QQueue<python_command> command_stack;
  /// @brief store the break points of the input, in order to send them when the execution is asked
  QVector<int> temporary_break_points;
  /// @brief a little weird but it is for avoid cross call when they are pending commands on the command stack
  QTimer auto_execution_timer;
  /// @brief store the state of the execution
  bool stopped;
  /// @brief ...
  bool text_being_entered;
  /// @brief used to add a PythonCodeEditor from the PythonConsole
  MainWindow *main_window;
  /// @brief to acces to the history list widget
  CustomListWidget *history_list_widget;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief simple inherited class to reimplement some behavior
class CustomListWidget : public QListWidget {
public:
  /// @brief constructor
  CustomListWidget(QWidget* parent=0) : QListWidget(parent) {}
protected:
  /// @brief ignore of draggable things that could go on the history list
  Qt::DropActions	supportedDropActions() const {
    return Qt::IgnoreAction;
  }
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonConsole_hpp
