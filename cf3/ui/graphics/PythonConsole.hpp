// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_ui_graphics_PythonConsole_hpp
#define cf3_ui_graphics_PythonConsole_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QKeyEvent>
#include <QStringList>
#include <QCompleter>
#include <QQueue>
#include <QPair>
#include "ui/graphics/PythonCodeContainer.hpp"

#include "ui/graphics/LibGraphics.hpp"

class QToolBar;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

class Graphics_API PythonConsole : public PythonCodeContainer
{
    Q_OBJECT
public:
    PythonConsole(QWidget *parent = 0);
    ~PythonConsole();
    void key_press_event(QKeyEvent *);
    void new_line(int indent_number);

private slots:
    void cursor_position_changed();
    void insert_output(const QString &);
    void insert_log(const QString &);
    void line_by_line_activated(bool);
    void stop_continue_pressed();
    void execution_stopped();
    void execute_code(QString,bool);
private:
    /// Index of the block that contains the current prompt
    int input_block;

    int input_start_in_text;

    void print_next_prompt();
    void push_input();
    void execute_input(QTextCursor &);
    void set_input(const QString &);
    void select_input(QTextCursor &);
    void fix_prompt_history();
    void stream_next_command();

    QAction* stop_continue;
    QStringList history;

    QQueue<QPair<QString,bool> > command_stack;
    int history_index;
    bool stopped;

    int output_line_number;
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonConsole_hpp
