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
#include <QListWidget>
#include <QWidget>
#include <QKeyEvent>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

////////////////////////////////////////////////////////////////////////////

PythonConsole::PythonConsole(QWidget *parent,MainWindow* main_window) :
  PythonCodeContainer(parent),main_window(main_window)
{
  python_console=this;
  document()->lastBlock().setUserState(PROMPT_1);
  history_index=0;
  input_start_in_text=0;
  input_block=0;
  text_being_entered=false;

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

  debug_arrow=-1;
  //history list
  history_list_widget=new CustomListWidget(this);
  history_list_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  history_list_widget->setDragDropMode(QAbstractItemView::DragOnly);

  connect(&auto_execution_timer,SIGNAL(timeout()),this,SLOT(stream_next_command()));
  connect(line_by_line,SIGNAL(toggled(bool)),this,SLOT(line_by_line_activated(bool)));
  connect(stop_continue,SIGNAL(triggered()),this,SLOT(stop_continue_pressed()));
  connect(break_action,SIGNAL(triggered()),this,SLOT(break_pressed()));
  connect(history_to_text_editor,SIGNAL(triggered()),this,SLOT(push_history_to_script_editor()));
  connect(ui::core::NScriptEngine::global().get(),SIGNAL(debug_trace_received(int,int)),this,SLOT(execution_stopped(int,int)));
  connect(core::NLog::global().get(), SIGNAL(new_message(QString, uiCommon::LogMessage::Type)),this,SLOT(insert_log(QString)));
  connect(core::NScriptEngine::global().get(),SIGNAL(execute_code_request(QString)),this,SLOT(execute_code(QString)));
  connect(core::NScriptEngine::global().get(),SIGNAL(append_false_command_request(QString)),this,SLOT(append_false_code(QString)));
  connect(core::NScriptEngine::global().get(),SIGNAL(debug_trace_received(int,int)),this,SLOT(display_debug_trace(int,int)));
  connect(core::NScriptEngine::global().get(),SIGNAL(change_fragment_request(int,int)),this,SLOT(change_code_fragment(int,int)));
  connect(this,SIGNAL(cursorPositionChanged()),this,SLOT(cursor_position_changed()));
  setViewportMargins(border_width,tool_bar->height(),0,0);
  offset_border.setX(border_width);
  offset_border.setY(tool_bar->height());
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::create_python_area(QWidget* widget){
  QHBoxLayout *main_layout=new QHBoxLayout(widget);
  widget->setLayout(main_layout);
  QSplitter *splitter=new QSplitter(Qt::Vertical,widget);
  main_layout->addWidget(splitter);
  QToolBar *tool_bar=new QToolBar(widget);
  tool_bar->setOrientation(Qt::Vertical);
  tool_bar->setAllowedAreas(Qt::RightToolBarArea);
  tool_bar->setFloatable(false);
  tool_bar->setMovable(false);
  main_layout->addWidget(tool_bar);
  QAction *python_scope_action=tool_bar->addAction("S\nc\no\np\ne");
  QAction *python_history_action=tool_bar->addAction("H\ni\ns\nt\no\nr\ny");
  python_scope_action->setCheckable(true);
  python_history_action->setCheckable(true);
  python_scope_action->setChecked(true);
  python_history_action->setChecked(true);
  connect(python_scope_action,SIGNAL(toggled(bool)),python_scope_values,SLOT(setVisible(bool)));
  connect(python_history_action,SIGNAL(toggled(bool)),history_list_widget,SLOT(setVisible(bool)));
  splitter->addWidget(python_scope_values);
  splitter->addWidget(history_list_widget);
  python_scope_values->setHidden(false);
  history_list_widget->setHidden(false);
  connect(python_scope_values,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(scope_double_click(QModelIndex)));
  connect(history_list_widget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(history_double_click(QModelIndex)));
}

void PythonConsole::cursor_position_changed(){
  if (text_being_entered){
    QTextCursor c=textCursor();
    if (!editable_zone(c)){
      c.setPosition(input_start_in_text);
      setTextCursor(c);
    }
  }
}

////////////////////////////////////////////////////////////////////////////

/*void PythonConsole::mousePressEvent(QMouseEvent *e){
  if (e->button() == Qt::MidButton){
    QTextCursor c=textCursor();
    if (!editable_zone(c)){
      c.movePosition(QTextCursor::End);
      setTextCursor(c);
    }
    text_being_entered=true;
    QPlainTextEdit::mousePressEvent(e);
    text_being_entered=false;
    fix_prompt();
  }else{
    QPlainTextEdit::mousePressEvent(e);
  }
}*/

//////////////////////////////////////////////////////////////////////////

int PythonConsole::get_debug_arrow_block(){
  return debug_arrow;
}

//////////////////////////////////////////////////////////////////////////
void PythonConsole::display_debug_trace(int fragment,int line){
  if (fragment > 0){
    int fragment_bloc_number=fragment_container[fragment];
    reset_debug_trace();
    debug_arrow=fragment_bloc_number+(line-1);
    QTextBlock block=document()->findBlockByNumber(debug_arrow);
    QTextCursor prev_cursor=textCursor();
    QTextCursor cursor(document());
    cursor.setPosition(block.position());
    setTextCursor(cursor);
    centerCursor();
    setTextCursor(prev_cursor);
    document()->markContentsDirty(block.position(),1);
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonConsole::register_fragment(QString code,int block_number,QVector<int> break_point){
  fragment_container.insert(++fragment_generator,block_number);
  blocks_fragment.insert(block_number,fragment_generator);
  ui::core::NScriptEngine::global().get()->execute_line(code,fragment_generator,break_point);
}

//////////////////////////////////////////////////////////////////////////

void PythonConsole::reset_debug_trace(){
  if (debug_arrow > -1){
    document()->markContentsDirty(document()->findBlockByNumber(debug_arrow).position(),1);
    debug_arrow=-1;
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonConsole::change_code_fragment(int fragment,int new_fragment){
  int block_number=fragment_container.value(fragment);
  if (block_number!=0){
    fragment_container.remove(fragment);
    fragment_container.insert(new_fragment,block_number);
    blocks_fragment.insert(block_number,new_fragment);
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::send_toggle_break_point(int fragment_block,int line_number){
  ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::TOGGLE_BREAK_POINT,blocks_fragment.value(fragment_block),line_number);
}

void PythonConsole::key_press_event(QKeyEvent *e){
  QTextCursor c=textCursor();
  if (e->text().length() > 0 && !editable_zone(c)){
    c.movePosition(QTextCursor::End);
    setTextCursor(c);
  }
  int block_count=blockCount();
  switch(e->key()){
  case Qt::Key_Backspace:
    if (c.selectedText().length() > 0 && (c.position() == input_start_in_text || c.anchor() == input_start_in_text)){//special case
      c.removeSelectedText();
      break;
    }
  case Qt::Key_Left:
    if ((c.position() - c.block().position()) > 0 || c.blockNumber() > input_block){
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
    if (c.blockNumber() == input_block && history.size()){
      if (history_index==history.size()){
        select_input(c);
        tmp_command=c.selectedText();
        tmp_command.replace(8233,'\n');
      }
      history_index--;
      if (history_index < 0)
        history_index=0;
      select_input(c);
      c.removeSelectedText();
      c.insertText(history.at(history_index));
      //centerCursor();
      fix_prompt();
    }else{
      QPlainTextEdit::keyPressEvent(e);
    }
    break;
  case Qt::Key_Down:
    if (c.blockNumber() == document()->blockCount()-1 && history.size()){
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
        fix_prompt();
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
    fix_prompt();
}

////////////////////////////////////////////////////////////////////////////

bool PythonConsole::editable_zone(const QTextCursor &cursor){
  return cursor.anchor() >= input_start_in_text && cursor.position() >= input_start_in_text;
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::insert_text(const QString &text){
  execute_code(text,true,QVector<int>());
}

////////////////////////////////////////////////////////////////////////////

bool PythonConsole::is_stopped(){
  return stopped;
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::fix_prompt(){
  QTextBlock block=document()->findBlockByNumber(input_block);
  if (block.userState() != PROMPT_1){
    block.setUserState(PROMPT_1);
    highlighter->rehighlightBlock(block);
  }
  block=block.next();
  while (block.isValid()){
    if (block.userState() != PROMPT_2){
      block.setUserState(PROMPT_2);
      highlighter->rehighlightBlock(block);
    }
    block=block.next();
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::new_line(int indent_number){
  QTextCursor c=textCursor();
  c.movePosition(QTextCursor::EndOfBlock);
  if (c.block().userState() != PROMPT_1 || c.block().length() > 1){
    c.insertText("\n");
    if (indent_number>0){
      for (int i=0;i<indent_number;i++)
        c.insertText("\t");
    }
    setTextCursor(c);
    if (indent_number==0){//single line statement
      QTextCursor c=textCursor();
      execute_input(c);
    }else{//multi line
      fix_prompt();
    }
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
  command.replace(QChar(8233),'\n');
  while (command[command.size()-1] == '\n')
    command.chop(1);
  if (command.size()){
    register_fragment(command,input_block,temporary_break_points);
    history.append(command);
    add_history_draggable_item(command);
    history_index=history.size();
    c.movePosition(QTextCursor::End);
    input_block=c.blockNumber();
    input_start_in_text=c.position();
    //centerCursor();
    document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_1);
    if (command_stack.size())
      auto_execution_timer.start();//to avoid cross call with stream_next_command
  }
  temporary_break_points.clear();
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::execute_code(QString code,bool immediate,QVector<int> break_lines){
  static QRegExp two_points("^[^#:]*:[ ]*(#[^$]*)?$");
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

void PythonConsole::execute_code(QString code){
  execute_code(code,true,QVector<int>());
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
    c.insertText(line);
    c.movePosition(QTextCursor::End);
    c.insertBlock();
    if (document()->lastBlock().userState() != PROMPT_1)
      document()->lastBlock().setUserState(PROMPT_2);
  }
  setTextCursor(c);
  //centerCursor();
  fix_prompt();
  if (current_command.imediate){
    execute_input(c);
  }
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::insert_output(QString output, int fragment){
  output.replace("\\n","\n");
  QTextCursor cursor=textCursor();
  if (fragment < 0){
    if (input_start_in_text==0){//special case
      cursor.setPosition(0);
      cursor.insertText("\n");
      document()->findBlockByNumber(0).setUserState(1);
      document()->findBlockByNumber(1).setUserState(PythonCodeContainer::PROMPT_1);
      cursor.setPosition(0);
    }else{
      cursor.setPosition(input_start_in_text);
      cursor.movePosition(QTextCursor::Left);
      cursor.insertBlock();
    }
  }else{
    int b=fragment_container[fragment];
    QTextBlock bl;
    if (b == 0){
      bl=document()->findBlockByNumber(input_block);
    }else{
      bl=document()->findBlockByNumber(b);
      bl=bl.next();
    }
    while (bl.isValid()){
      if (bl.userState() == PROMPT_1){
        cursor.setPosition(bl.position());
        cursor.movePosition(QTextCursor::Left);
        cursor.insertBlock();
        break;
      }
      bl=bl.next();
    }
  }
  QTextCursor insert_cursor(cursor);
  int pos_c=cursor.position();
  cursor.insertText(output);
  cursor.movePosition(QTextCursor::Right);
  input_start_in_text+=output.size()+1;
  insert_cursor.setPosition(pos_c);
  //cursor.setPosition(insert_cursor.position()+);
  int block_diff=(cursor.blockNumber()-insert_cursor.blockNumber());
  input_block+=block_diff;
  QTextBlock block=insert_cursor.block().previous();
  int output_line_number;
  if (!block.isValid()){
    output_line_number=1;
  }else{
    output_line_number=block.userState()+1;
    if (output_line_number > PROMPT_1 || output_line_number == 0)
      output_line_number=1;
  }
  block=insert_cursor.block();
  while (block.blockNumber() < cursor.blockNumber()){
    if (block.userState() < 0)
      block.setUserState(output_line_number++);
    block=block.next();
  }
  if (fragment > 0){
    QMap<int, int>::const_iterator itt_fragment=fragment_container.begin();
    while (itt_fragment != fragment_container.end()){
      if (itt_fragment.key() > fragment){
        fragment_container[itt_fragment.key()]=itt_fragment.value()+block_diff;
        blocks_fragment.remove(itt_fragment.value());
      }
      ++itt_fragment;
    }
    itt_fragment=fragment_container.begin();
    while (itt_fragment != fragment_container.end()){
      if (itt_fragment.key() > fragment){
        blocks_fragment.insert(itt_fragment.value(),itt_fragment.key());
      }
      ++itt_fragment;
    }
  }
  ensureCursorVisible();
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::insert_log(const QString &output){
  static QRegExp log_frame("\\[ [^\\]]* \\]\\[ [^\\]]* \\] (Worker\\[0\\] )?");
  static QRegExp fragment_frame(":[0-9]+:");
  int start=log_frame.indexIn(output);
  int fragment=-1;
  QString insert_string;
  while (start>=0){
    int rstart=start+log_frame.matchedLength();
    start=log_frame.indexIn(output,rstart);
    QString modified_output;
    if (start >=0)
      modified_output=output.mid(rstart,start-rstart);
    else
      modified_output=output.mid(rstart);
    int st=fragment_frame.indexIn(modified_output);
    int ml;
    if ((ml=fragment_frame.matchedLength()) != -1){
      fragment=modified_output.mid(st+1,ml-2).toInt();
      insert_string.append(modified_output.mid(st+ml));
    }else
      insert_string.append(modified_output);
  }
  insert_output(insert_string,fragment);
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::append_false_code(QString code){
  int ind=code.indexOf('\n');
  QString first_line=code.mid(0,ind);
  QTextCursor c(document());
  c.setPosition(input_start_in_text);
  c.insertBlock();
  c.movePosition(QTextCursor::Left);
  c.insertText(first_line);
  c.block().setUserState(PROMPT_1);
  input_start_in_text+=first_line.size()+1;
  input_block+=1;
  history.append(first_line);
  add_history_draggable_item(first_line);
  history_index=history.size();
  fix_prompt();
  if (ind > -1){
    append_false_code(code.mid(ind+1));
  }
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

void PythonConsole::add_history_draggable_item(const QString & text){
  QListWidgetItem* item=new QListWidgetItem(text,history_list_widget,history_list_widget->count());
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
  history_list_widget->addItem(item);
}

////////////////////////////////////////////////////////////////////////////

void PythonConsole::history_double_click(const QModelIndex & index){
  execute_code(history_list_widget->item(index.row())->text(),false,QVector<int>());
}

////////////////////////////////////////////////////////////////////////////

const QListWidget* PythonConsole::get_history_list_widget(){
  return history_list_widget;
}

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
