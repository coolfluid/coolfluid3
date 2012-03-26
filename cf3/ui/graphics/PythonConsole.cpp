// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "ui/core/NScriptEngine.hpp"
#include "ui/core/NLog.hpp"
#include "ui/graphics/PythonConsole.hpp"
#include "ui/graphics/PythonCodeEditor.hpp"
#include "ui/graphics/MainWindow.hpp"
#include "common/Log.hpp"
#include <QStringListModel>
#include <QMessageBox>
#include <QTextBlock>
#include <QScrollBar>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QToolBar>
#include <QTableWidget>
#include <QAction>
#include <QTreeView>
#include <QSplitter>
#include <QTabWidget>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

////////////////////////////////////////////////////////////////////////////

PythonConsole::PythonConsole(QWidget *parent,MainWindow* main_window) :
  PythonCodeContainer(parent),main_window(main_window)
{
  python_console=this;
  document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_1);
  history_index=0;
  input_start_in_text=0;
  output_line_number=1;
  input_block=0;

  setUndoRedoEnabled(false);

  //Toolbar
  line_by_line=new QAction(QIcon(":/Icons/action_line_by_line.png"),"Line by line",this);
  line_by_line->setCheckable(true);
  tool_bar->addAction(line_by_line);
  stop_continue=new QAction(QIcon(":/Icons/debug_arrow.png"),"Continue",this);
  stop_continue->setEnabled(false);
  stopped=false;
  tool_bar->addAction(stop_continue);
  auto_execution_timer.setInterval(0);
  auto_execution_timer.setSingleShot(true);
  break_action=new QAction(QIcon(":/Icons/action_break.png"),"Break",this);
  tool_bar->addAction(break_action);
  QAction* history_to_text_editor=new QAction(QIcon(":/Icons/action_new_script_from_history")
                                              ,"Create script from history",this);
  tool_bar->addAction(history_to_text_editor);
  connect(&auto_execution_timer,SIGNAL(timeout()),this,SLOT(stream_next_command()));
  connect(line_by_line,SIGNAL(toggled(bool)),this,SLOT(line_by_line_activated(bool)));
  connect(stop_continue,SIGNAL(triggered()),this,SLOT(stop_continue_pressed()));
  connect(break_action,SIGNAL(triggered()),this,SLOT(break_pressed()));
  connect(history_to_text_editor,SIGNAL(triggered()),this,SLOT(push_history_to_script_editor()));
  connect(ui::core::NScriptEngine::global().get(),SIGNAL(debug_trace_received(int,int)),this,SLOT(execution_stopped(int,int)));
  connect(core::NScriptEngine::global().get(),SIGNAL(new_output(QString)),this,SLOT(insert_output(QString)));
  connect(core::NLog::global().get(), SIGNAL(new_message(QString, uiCommon::LogMessage::Type)),
          this, SLOT(insert_log(QString)));
  connect(core::NScriptEngine::global().get(),SIGNAL(execute_code_request(QString,bool,QVector<int>&)),this,SLOT(execute_code(QString,bool,QVector<int>&)));
  setViewportMargins(border_width,tool_bar->height(),0,0);
  offset_border.setX(border_width);
  offset_border.setY(tool_bar->height());
}

////////////////////////////////////////////////////////////////////////////

PythonConsole::~PythonConsole(){

}

void PythonConsole::create_splitter(QTabWidget* tab_widget){
  QSplitter *splitter=new QSplitter(tab_widget);
  splitter->addWidget(this);
  splitter->addWidget(python_scope_values);
  tab_widget->addTab(splitter,"Python Console");
  connect(python_scope_values,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(scope_double_click(QModelIndex)));
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::key_press_event(QKeyEvent *e){
  QTextCursor c=textCursor();
  int block_count=blockCount();
  switch(e->key()){
  case Qt::Key_Backspace:
  case Qt::Key_Left:
    if (c.positionInBlock() > 0 || c.blockNumber() > input_block){
      QPlainTextEdit::keyPressEvent(e);
      c=textCursor();
      if (c.block().length() == 0
          && c.block().userState()==PythonCodeContainer::PROMPT_2
          && c.block()==document()->lastBlock()){
        //user remove all ident of the line we execute it
        new_line(0);
      }
    }
    break;
  case Qt::Key_Up:
    if (c.blockNumber() == input_block){
      if (history_index==history.size()){
        select_input(c);
        tmp_command=c.selectedText();
      }
      history_index--;
      if (history_index < 0)
        history_index=0;
      select_input(c);
      c.removeSelectedText();
      c.insertText(history.at(history_index));
      //centerCursor();
      fix_prompt_history();
    }else{
      QPlainTextEdit::keyPressEvent(e);
    }
    break;
  case Qt::Key_Down:
    if (c.blockNumber() == document()->blockCount()-1){
      if (history_index < history.size()){
        history_index++;
        select_input(c);
        c.removeSelectedText();
        if (history_index > history.size()-1){
          history_index=history.size();
          c.insertText(tmp_command);
        }else{
          c.insertText(history.at(history_index));
        }
        setTextCursor(c);
        fix_prompt_history();
      }
    }else{
      QPlainTextEdit::keyPressEvent(e);
    }
    break;
  case Qt::Key_Home:
    c.movePosition(QTextCursor::StartOfBlock);
    //c.movePosition(QTextCursor::WordRight);
    setTextCursor(c);
    break;
  default:
    QPlainTextEdit::keyPressEvent(e);
  }
  if (blockCount() != block_count)
    fix_prompt_history();
}

////////////////////////////////////////////////////////////////////////////

bool PythonConsole::editable_zone(const QTextCursor &cursor){
  return cursor.anchor() >= input_start_in_text && cursor.position() >= input_start_in_text;
}

////////////////////////////////////////////////////////////////////////////

bool PythonConsole::is_stopped(){
  return stopped;
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::fix_prompt_history(){
  QTextBlock block=document()->findBlockByNumber(input_block);
  block.setUserState(PythonCodeContainer::PROMPT_1);
  block=block.next();
  while (block.isValid()){
    block.setUserState(PythonCodeContainer::PROMPT_2);
    block=block.next();
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::new_line(int indent_number){
  if (indent_number==0){//single line statement
    QTextCursor c=textCursor();
    execute_input(c);
  }else{//multi line
    fix_prompt_history();
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::border_click(const QPoint &pos){
  QTextBlock b=cursorForPosition(pos-offset_border).block();
  int current_block=b.blockNumber();
  if (b.userState() < PROMPT_1)
    return;
  document()->markContentsDirty(b.position(),1);
  while (b.isValid() && b.userState() > PROMPT_1){
    b=b.previous();
  }
  int first_block=b.blockNumber();
  int line_number=current_block-first_block;
  if (current_block >= input_block){
    int ind=temporary_break_points.indexOf(line_number);
    if (ind > -1){
      temporary_break_points.remove(ind);
    }else{
      int i;
      for (i=0;i<temporary_break_points.size();i++)
        if (temporary_break_points[i] > line_number)
          break;
      temporary_break_points.insert(i,line_number);
    }
  }
  toggle_break_point(first_block,line_number,current_block<input_block);
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::execute_input(QTextCursor &c){
  select_input(c);
  QString command=c.selectedText();
  register_fragment(command,input_block,temporary_break_points);
  temporary_break_points.clear();
  command.chop(1);
  history.append(command);
  history_index=history.size();
  c.movePosition(QTextCursor::End);
  input_block=c.blockNumber();
  input_start_in_text=c.position();
  output_line_number=1;
  //centerCursor();
  document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_1);
  if (command_stack.size())
    auto_execution_timer.start();//to avoid cross call with stream_next_command
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::execute_code(QString code,bool immediate,QVector<int> &break_lines){
  static QRegExp two_points("^[^#:]*:");
  static QRegExp extend_statement("^[^#:]*(catch|elif|else|finally)[^#:]*:");
  static QRegExp empty_line("^[ \\t]*($|#)");
  QString command;
  QString line;
  QVector<int> current_break;
  int line_number=0;
  int block_number=0;
  int ind=-1;

  bool multi_line=false;

  foreach (line,code.split('\n')){
    if (ind=break_lines.indexOf(line_number) > -1){
      current_break.push_back(line_number-block_number);
    }
    if (multi_line){
      int indent=0;
      for (int i=0;(line[i]=='\t'|| (line[i]==' ' && line[++i]==' '));indent++,i++);//match tabulation and double space
      if (indent==0){
        if (line.contains(extend_statement)){
          command.append(line).append('\n');
        }else{//end of multi line commnd
          command_stack.enqueue(python_command(command,immediate,current_break));
          block_number=line_number;
          current_break.clear();
          command.clear();
          multi_line=false;
        }
      }else{
        command.append(line).append('\n');
      }
    }
    if (!multi_line){
      if (line.contains(two_points)){
        multi_line=true;
        command=line.append('\n');
      }else{//simple command
        if (!line.contains(empty_line)){
          command_stack.enqueue(python_command(line,immediate,current_break));
          block_number=line_number;
          current_break.clear();
        }
      }
    }
    line_number++;
  }
  if (command_stack.size()){
    stream_next_command();
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::stream_next_command(){
  python_command current_command=command_stack.dequeue();
  QTextCursor c=textCursor();
  select_input(c);
  c.removeSelectedText();
  temporary_break_points=current_command.break_lines;
  for (int i=0;i<temporary_break_points.size();i++){
    toggle_break_point(input_block,temporary_break_points[i],false);
  }
  QString line;
  foreach (line,current_command.command.split('\n')){
    c.insertText(line.append('\n'));
    c.movePosition(QTextCursor::End);
    document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_2);
  }
  setTextCursor(c);
  //centerCursor();
  if (current_command.imediate){
    execute_input(c);
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::insert_output(const QString &output){
  QTextCursor cursor=textCursor();
  if (input_start_in_text==0){//special case
    cursor.setPosition(0);
    cursor.insertText("\n");
    document()->findBlockByNumber(0).setUserState(-1);
    document()->findBlockByNumber(1).setUserState(PythonCodeContainer::PROMPT_1);
    cursor.setPosition(0);
  }else{
    cursor.setPosition(input_start_in_text);
    cursor.movePosition(QTextCursor::Left);
    cursor.insertBlock();
  }
  cursor.insertText(output);
  input_start_in_text+=output.size()+1;
  cursor.setPosition(input_start_in_text);
  QTextBlock block=document()->findBlockByNumber(input_block-1);
  input_block=cursor.blockNumber();
  while (block.isValid()){
    if (block.userState() < 0)
      block.setUserState(output_line_number++);
    block=block.next();
  }
  ensureCursorVisible();
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::insert_log(const QString &output){
  static QRegExp log_frame("\\[ [^\\]]* \\]\\[ [^\\]]* \\] (Worker\\[0\\] )?");
  QString modified_output;
  int start=log_frame.indexIn(output);
  while (start>=0){
    int rstart=start+log_frame.matchedLength();
    start=log_frame.indexIn(output,rstart);
    if (start >=0){
      modified_output.append(output.mid(rstart,start-rstart));
    }else{
      modified_output.append(output.mid(rstart));
    }
  }
  modified_output.replace("\\n","\n");
  insert_output(modified_output);
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::select_input(QTextCursor &cursor){
  cursor.setPosition(input_start_in_text);
  cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::line_by_line_activated(bool activated){
  CFinfo << (activated?"true":"false") << CFendl;
  if (activated){
    ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::LINE_BY_LINE_EXECUTION);
    if (python_scope_values->isColumnHidden(1)){
      python_scope_values->setColumnHidden(1,false);
      python_scope_values->setColumnWidth(0,python_scope_values->columnWidth(0)-150);
    }
  }else{
    ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::NORMAL_EXECUTION);
    python_scope_values->setColumnHidden(1,true);
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::stop_continue_pressed(){
  ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::CONTINUE);
  reset_debug_trace();
  break_action->setEnabled(false);
  stop_continue->setEnabled(false);
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::break_pressed(){
  ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::BREAK);
  reset_debug_trace();
  break_action->setEnabled(false);
  stop_continue->setEnabled(false);
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::execution_stopped(int fragment,int line){
  Q_UNUSED(fragment);Q_UNUSED(line);
  break_action->setEnabled(true);
  stop_continue->setEnabled(true);
  if (python_scope_values->isColumnHidden(1)){
    python_scope_values->setColumnHidden(1,false);
    python_scope_values->setColumnWidth(0,python_scope_values->columnWidth(0)-150);
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::push_history_to_script_editor(){
  PythonCodeEditor * editor=main_window->create_new_python_editor();
  editor->setPlainText(history.join("\n"));
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::scope_double_click(const QModelIndex & index){
  QStandardItem* item=python_dictionary.itemFromIndex(index);
  QString command=item->text();
  item=item->parent();
  while (item){
    command.prepend(item->text()+".");
    item=item->parent();
  }
  textCursor().insertText(command);
}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
