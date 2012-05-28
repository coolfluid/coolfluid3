// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "ui/graphics/PythonCodeContainer.hpp"
#include "ui/core/NScriptEngine.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/ThreadManager.hpp"
#include "ui/core/TreeThread.hpp"
#include "ui/graphics/PythonConsole.hpp"
#include "common/Log.hpp"
#include <QStringListModel>
#include <QMessageBox>
#include <QTextBlock>
#include <QScrollBar>
#include <QRegExp>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>
#include <QTreeView>
#include <QProxyStyle>
#include <QListWidget>
#include <QKeyEvent>
#include <QPainter>
#include <QToolTip>
#include <QBitmap>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

PythonCompleter* PythonCodeContainer::completer=NULL;
PythonConsole* PythonCodeContainer::python_console=NULL;
QTreeView* PythonCodeContainer::python_scope_values=NULL;
QVector<PythonCodeContainer::PythonDict> PythonCodeContainer::dictionary;
QStandardItemModel PythonCodeContainer::python_dictionary;
QPixmap* BorderArea::debug_arrow=NULL;
QPixmap* BorderArea::break_point=NULL;

//////////////////////////////////////////////////////////////////////////

PythonCodeContainer::PythonCodeContainer(QWidget *parent) :
  QPlainTextEdit(parent)
{
  if (completer==NULL){//init static member
    QStringList headers;
    headers << "Scope keys" << "Scope values";
    python_dictionary.setHorizontalHeaderLabels(headers);
    completer=new PythonCompleter(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseSensitive);
    completer->setModel(&python_dictionary);
    python_scope_values=new QTreeView(this);
    python_scope_values->setModel(&python_dictionary);
    python_scope_values->setWindowFlags(Qt::WindowStaysOnTopHint);
    python_scope_values->setAttribute(Qt::WA_DeleteOnClose,false);
    python_scope_values->setEditTriggers(QAbstractItemView::NoEditTriggers);
    python_scope_values->setColumnHidden(1,true);
    connect(core::ThreadManager::instance().tree().root().get(),SIGNAL(connected())
            ,core::NScriptEngine::global().get(),SLOT(client_connected()));
    connect(core::NScriptEngine::global().get(),SIGNAL(completion_list_received(QStringList,QStringList)),
            this,SLOT(keywords_changed(QStringList,QStringList)));
  }
  setAcceptDrops(true);
  highlighter=new PythonSyntaxeHighlighter(document());
  setFont(QFont("Monospace"));
  border_width=fontMetrics().width(QLatin1Char('>'))*3+20;
  border_area=new BorderArea(this,border_width);
  setLineWrapMode(QPlainTextEdit::WidgetWidth);
  tool_bar=new QToolBar(this);
  tool_bar->setMovable(false);
  tool_bar->setFloatable(false);
  tool_bar->setIconSize(QSize(16,16));
  //tool_bar->setLayout(new QHBoxLayout());
  //tool_bar->layout()->setContentsMargins(0,0,0,0);
  //tool_bar->setFixedHeight(42);
  QHBoxLayout *layout=new QHBoxLayout;
  layout->setContentsMargins(0,0,0,0);
  //tool_bar->setFixedHeight(tool_bar->height());
  layout->addSpacerItem(new QSpacerItem(border_width,0));
  layout->addWidget(tool_bar);
  layout->setAlignment(tool_bar,Qt::AlignTop);
  //layout->setAlignment(tool_bar,Qt::AlignTop);
  setLayout(layout);
  setViewportMargins(border_width,tool_bar->height(),0,0);
  setTabStopWidth(fontMetrics().width(QLatin1Char(' '))*2);
  offset_border.setX(border_width);
  offset_border.setY(tool_bar->height());
  //this border is used to display line number or the prompt
  doc_timer.setInterval(400);
  doc_timer.setSingleShot(true);
  setMouseTracking(true);
  connect(this,SIGNAL(updateRequest(QRect,int)),this,SLOT(update_border_area(QRect,int)));
  connect(&doc_timer,SIGNAL(timeout()),this,SLOT(request_documentation()));
  connect(core::NScriptEngine::global().get(), SIGNAL(documentation_received(QString)), this,SLOT(popup_documentation(QString)));
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::update_border_area(const QRect &rect,int dy){
  if (dy){
    border_area->scroll(0,dy);
  }else{
    border_area->update(0,rect.y()+tool_bar->height(),border_width,rect.height()+tool_bar->height());
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::repaint_border_area(QPaintEvent *event){
  if (BorderArea::debug_arrow==NULL){
    BorderArea::break_point=new QPixmap(":/Icons/break_point.png");
    BorderArea::debug_arrow=new QPixmap(":/Icons/debug_arrow.png");
  }
  QPainter painter(border_area);
  painter.fillRect(event->rect(), Qt::lightGray);
  QTextBlock block = firstVisibleBlock();
  int block_number = block.blockNumber();
  int vertical_displace=tool_bar->height();
  int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top()+vertical_displace;
  int bottom = top + (int) blockBoundingRect(block).height();
  int debug_arrow_block=python_console->get_debug_arrow_block();
  painter.setPen(Qt::black);
  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      switch (block.userState()){
      case PROMPT_1://display prompt 1
        painter.drawText(0, top, border_width-4, fontMetrics().height(),
                         Qt::AlignRight, ">>>");
        break;
      case PROMPT_2://display prompt 2
        painter.drawText(0, top, border_width-4, fontMetrics().height(),
                         Qt::AlignRight, "...");
        break;
      case LINE_NUMBER://display bloc number
        painter.drawText(0, top, border_width-4, fontMetrics().height(),
                         Qt::AlignRight, QString::number(block_number + 1));
        break;
      default:
        painter.drawText(0, top, border_width-4, fontMetrics().height(),
                         Qt::AlignRight, QString::number(block.userState()));
      }
      int break_point_index=break_points.indexOf(block_number);
      if (break_point_index!=-1)
        painter.drawPixmap(0,top,16,16,*BorderArea::break_point);
      if (block_number == debug_arrow_block)
        painter.drawPixmap(2,top,16,16,*BorderArea::debug_arrow);
    }
    block = block.next();
    top = bottom;
    bottom = top + (int) blockBoundingRect(block).height();
    block_number++;
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::toggle_break_point(int fragment_block, int line_number,bool send){
  if (send)
    python_console->send_toggle_break_point(fragment_block,line_number);
  int block_number=fragment_block+line_number;
  int ind=break_points.indexOf(block_number);
  if (ind > -1){
    break_points.remove(ind);
  }else{
    int i;
    for (i=0;i<break_points.size();i++)
      if (break_points[i] > block_number)
        break;
    break_points.insert(i,block_number);
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::resizeEvent(QResizeEvent *e){
  QPlainTextEdit::resizeEvent(e);
  QRect cr=contentsRect();
  border_area->setGeometry(cr.left(),cr.top(),border_width,cr.bottom());
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::keyPressEvent(QKeyEvent *e){
  static QRegExp indent_case("^[^#:]*:[ ]*(#[^$]*)?$");
  QTextCursor c=textCursor();
  QTextCursor temp;
  if (editable_zone(c)){
    if (completer != NULL && completer->popup() != NULL && completer->popup()->isVisible()){
      switch(e->key()){
      case Qt::Key_Backspace:
        if (editable_zone(c)){
          temp=c;temp.movePosition(QTextCursor::Left);
          if (editable_zone(temp)){
            QPlainTextEdit::keyPressEvent(e);
            break;
          }
        }
      case Qt::Key_Enter:
      case Qt::Key_Escape:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
      case Qt::Key_Return:
        e->ignore();
        return;
      default:
        QPlainTextEdit::keyPressEvent(e);
      }
      QString word_under_cursor=get_word_under_cursor(c);
      QAbstractItemView* popup=completer->popup();
      if (e->text().size()>=0){//text entered
        //completer->setModel(new QStringListModel(generate_dictionary(word_under_cursor)));
        completer->setCompletionPrefix(word_under_cursor);
        popup->setCurrentIndex(completer->completionModel()->index(0,0));
        QRect completer_rect=cursorRect(c);
        completer_rect.moveTopLeft(completer_rect.topLeft()+offset_border);
        completer_rect.setWidth(popup->sizeHintForColumn(0)
                                +popup->verticalScrollBar()->sizeHint().width());
        completer->complete(completer_rect);
      }
    }else{
      if (e->key()==Qt::Key_Return){//auto indentation
        QTextCursor c=textCursor();
        QString current_line=document()->findBlockByNumber(c.blockNumber()).text();
        int tab_number=0;
        for (int i=0;i<current_line.size();i++){
          if (current_line[i]=='\t')
            tab_number++;
          else
            break;
        }
        if (indent_case.indexIn(current_line) > -1)
          tab_number++;
        new_line(tab_number);
      }else if (e->key()==Qt::Key_Escape){
        QString word_under_cursor=get_word_under_cursor(c);
        if (word_under_cursor.length() >= 0){
          setTextCursor(c);
          if (static_cast<PythonCodeContainer*>(completer->widget()) != NULL)
            disconnect(completer,SIGNAL(activated(QString)),static_cast<PythonCodeContainer*>(completer->widget()),SLOT(insert_completion(QString)));
          completer->setWidget(this);
          //completer->setModel(new QStringListModel(generate_dictionary(word_under_cursor)));
          completer->setCompletionPrefix(word_under_cursor);
          QAbstractItemView* popup=completer->popup();
          popup->setCurrentIndex(completer->completionModel()->index(0,0));
          QRect completer_rect=cursorRect();
          completer_rect.moveTopLeft(completer_rect.topLeft()+offset_border);
          completer_rect.setWidth(popup->sizeHintForColumn(0)
                                  +popup->verticalScrollBar()->sizeHint().width());
          completer->complete(completer_rect);
          connect(completer,SIGNAL(activated(QString)),this,SLOT(insert_completion(QString)));
        }
      }else{
        key_press_event(e);
      }
    }
  }else{//allow copy in uneditable zones
    if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_C)
      QPlainTextEdit::keyPressEvent(e);
    else
      key_press_event(e);
  }
  ensureCursorVisible();
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::mouseMoveEvent(QMouseEvent *e){
  doc_timer.stop();
  doc_timer.start();
  last_mouse_pos=e->pos();
  QToolTip::hideText();
  QPlainTextEdit::mouseMoveEvent(e);
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::leaveEvent(QEvent *e){
  doc_timer.stop();
  QToolTip::hideText();
  QPlainTextEdit::leaveEvent(e);
}

//////////////////////////////////////////////////////////////////////////

QMimeData* PythonCodeContainer::createMimeDataFromSelection() const{
  QMimeData* mime_data=new QMimeData();
  mime_data->setText(textCursor().selectedText());
  return mime_data;
}

//////////////////////////////////////////////////////////////////////////

bool PythonCodeContainer::canInsertFromMimeData(const QMimeData *source) const{
  if (source->hasText() || source->hasFormat("application/x-qabstractitemmodeldatalist"))
    return true;
  return false;
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::insertFromMimeData(const QMimeData *source){
  if (source->hasText())
    insert_text(source->text());
  if (source->hasFormat("application/x-qabstractitemmodeldatalist")){
    QString text;
    QList<const QListWidgetItem*> sorted_items;
    foreach (const QListWidgetItem* item, python_console->get_history_list_widget()->selectedItems()){
      QList<const QListWidgetItem*>::iterator itt=sorted_items.begin();
      for (;itt != sorted_items.end();itt++){
        if ((*itt)->type() > item->type())
          break;
      }
      sorted_items.insert(itt,item);
    }
    foreach (const QListWidgetItem* item, sorted_items){
      text.append(item->text()+"\n");
    }
    insert_text(text);
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::insert_completion(QString completion){
  QTextCursor cursor=textCursor();
  cursor.insertText(completion.right(completion.length()-completer->completionPrefix().length()));
  setTextCursor(cursor);
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::keywords_changed(const QStringList &add, const QStringList &sub){

  /*std::cout << "add" << std::endl;
  for (int i=0;i<add.size();i++){
    std::cout << add.at(i).toStdString() << std::endl;
  }
  std::cout << "sub" << std::endl;
  for (int i=0;i<sub.size();i++){
    std::cout << sub.at(i).toStdString() << std::endl;
  }*/
  for (int i=0;i<sub.size();i++){
    if (sub[i]=="*"){
      python_dictionary.removeRows(0,python_dictionary.rowCount());
    }else{
      remove_dictionary_item(sub[i],(CustomStandardItem*)python_dictionary.invisibleRootItem());
    }
  }
  int i=0;
  add_to_dictionary(i,add,(CustomStandardItem*)python_dictionary.invisibleRootItem());
  python_dictionary.sort(0);
}

void PythonCodeContainer::add_to_dictionary(int &i,const QStringList &add,CustomStandardItem *item){
  while(i<add.size()){
    QString s=add[i];
    QChar c=s[s.size()-1];
    if (c=='{' || c==':'){
      s.chop(1);
      int two_point=s.indexOf(':');
      QString item_path=s.mid(0,two_point);
      CustomStandardItem *n_value;
      if (two_point != -1)
        n_value=new CustomStandardItem(s.mid(two_point+1));
      else
        n_value=new CustomStandardItem("");
      CustomStandardItem *current_item=item;
      QStringList path_list=item_path.split('.');
      for (int j=0;j<path_list.size()-1;j++){
        for (int k=0;k<current_item->rowCount();k++){
          CustomStandardItem *n_item=(CustomStandardItem*)current_item->child(k);
          if (n_item->text()==path_list[j]){
            current_item=n_item;
          }
        }
      }
      CustomStandardItem *n_item=new CustomStandardItem(path_list.last());
      current_item->appendRow(n_item);
      current_item->setChild(n_item->row(),1,n_value);
      i++;
      if (c!=':')
        add_to_dictionary(i,add,n_item);
    }else if (c=='}'){
      i++;
      return;
    }else{
      CustomStandardItem *n_item=new CustomStandardItem(s);
      item->appendRow(n_item);
      i++;
    }
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::remove_dictionary_item(QString name,CustomStandardItem *item){
  int p=name.indexOf('.');
  if (p > -1){
    QString start=name.mid(0,p);
    QString end=name.mid(p+1);
    for (int i=0;i<item->rowCount();i++){
      CustomStandardItem *n_item=(CustomStandardItem*)item->child(i);
      QString t_str=n_item->text();
      if (t_str.size() && t_str[t_str.size()-1] == '('){
        t_str.chop(1);
      }
      if (n_item->text()==start){
        remove_dictionary_item(end,n_item);
      }
    }
  }else{
    for (int i=0;i<item->rowCount();i++){
      CustomStandardItem *n_item=(CustomStandardItem*)item->child(i);
      QString t_str=n_item->text();
      if (t_str.size() && t_str[t_str.size()-1] == '('){
        t_str.chop(1);
      }
      if (t_str==name){
        item->removeRow(n_item->row());
        i--;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::request_documentation(){
  if (!python_console->is_stopped()){
    QTextCursor c=cursorForPosition(last_mouse_pos);
    QTextBlock b=c.block();
    if (b.userData() == NULL){
      QString word=get_word_under_cursor(c);
      if (word.size() > 1){
        if (word != last_documented_word){
          last_documented_word=word;
          ui::core::NScriptEngine::global().get()->request_documentation(word);
        }else{
          popup_documentation(last_documentation);
        }
      }
    }else{
      popup_documentation(static_cast<TextBlockErrorData*>(b.userData())->get_error_string());
    }
  }
}

//////////////////////////////////////////////////////////////////////////

void PythonCodeContainer::popup_documentation(const QString & documentation){
  if (documentation.size()){
    last_documentation=documentation;
    last_documentation.replace("\\n","\n");
    QToolTip::showText(mapToGlobal(last_mouse_pos+offset_border),last_documentation,this);
  }
}

//////////////////////////////////////////////////////////////////////////

QString PythonCodeContainer::get_word_under_cursor(QTextCursor &c){
  QString block=c.block().text();
  static QRegExp complete_word("[\\w\\.]+");
  int position_in_block=c.position() - c.block().position();
  int index = complete_word.indexIn(block);
  while (index >= 0) {
    int length = complete_word.matchedLength();
    if (index < position_in_block && index+length >= position_in_block){
      c.setPosition(c.block().position()+index+length);
      return block.mid(index,length);
    }
    index = complete_word.indexIn(block, index + length);
  }
  return "";
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
