// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "PythonCodeContainer.hpp"
#include "ui/core/NScriptEngine.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NRoot.hpp"
#include "ui/core/ThreadManager.hpp"
#include "ui/core/TreeThread.hpp"
#include "common/Log.hpp"
#include <QStringListModel>
#include <QMessageBox>
#include <QTextBlock>
#include <QScrollBar>
#include <QRegExp>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

QMap<int,QPair<PythonCodeContainer*,int> > PythonCodeContainer::fragment_container=QMap<int,QPair<PythonCodeContainer*,int> >();
QCompleter* PythonCodeContainer::completer=NULL;
QPixmap* PythonCodeContainer::arrow_pixmap=NULL;
PythonCodeContainer* PythonCodeContainer::debug_arrow_container=NULL;
int PythonCodeContainer::fragment_generator=0;


PythonCodeContainer::PythonCodeContainer(QWidget *parent) :
    QPlainTextEdit(parent)
{
    if (completer==NULL){//init static member
        completer=new QCompleter(this);
        completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
        completer->setCaseSensitivity(Qt::CaseSensitive);
        //fragment_container.push_back(QPair<PythonCodeContainer*,int>(NULL,0));//dummy fragment, fragment index start at 1
        connect(core::ThreadManager::instance().tree().root().get(),SIGNAL(connected()),core::NScriptEngine::global().get(),SLOT(get_completion_list()));
        connect(core::NScriptEngine::global().get(),SIGNAL(completion_list_received(QStringList)),
                this,SLOT(keywords_changed(QStringList)));
        //connect(core::NScriptEngine::global().get(),SIGNAL(debug_trace_received(int,int)),this,SLOT(di)
    }
    highlighter=new PythonSyntaxeHighlighter(document());
    setFont(QFont("Monospace"));
    border_width=fontMetrics().width(QLatin1Char('>'))*5+5;
    border_area=new BorderArea(this,border_width);
    debug_arrow=-1;
    debug_arrow_container=NULL;
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    tool_bar=new QToolBar(this);
    QHBoxLayout *layout=new QHBoxLayout;
    tool_bar->setMinimumHeight(48);
    layout->addSpacerItem(new QSpacerItem(border_width,0));
    layout->addWidget(tool_bar);
    layout->setAlignment(tool_bar,Qt::AlignTop);
    //layout->setAlignment(tool_bar,Qt::AlignTop);
    setLayout(layout);
    setViewportMargins(border_width,tool_bar->height(),0,0);
    setTabStopWidth(fontMetrics().width(QLatin1Char('>'))*3);
    //this border is used to display line number or the prompt
    connect(this,SIGNAL(updateRequest(QRect,int)),this,SLOT(update_border_area(QRect,int)));
    connect(core::NScriptEngine::global().get(), SIGNAL(debug_trace_received(int,int)), this,SLOT(display_debug_trace(int,int)));
}

PythonCodeContainer::~PythonCodeContainer(){
    //remove_fragments();
}

void PythonCodeContainer::update_border_area(const QRect &rect,int dy){
    if (dy){
        border_area->scroll(0,dy);
    }else{
        border_area->update(0,rect.y()+tool_bar->height(),border_width,rect.height()+tool_bar->height());
    }
}

void PythonCodeContainer::register_fragment(QString code,int block_number){
    fragment_container.insert(++fragment_generator,QPair<PythonCodeContainer*,int>(this,block_number));
    ui::core::NScriptEngine::global().get()->execute_line(code,fragment_generator);
}

void PythonCodeContainer::remove_fragments(){
    for (QMap<int,QPair<PythonCodeContainer*,int> >::iterator i=fragment_container.begin();i!=fragment_container.end();i++){
        if (i.value().first == this){
            fragment_container.remove(i.key());
        }
    }
}

void PythonCodeContainer::display_debug_trace(int fragment,int line){
    if (fragment != -1){
        QPair<PythonCodeContainer*,int> p=fragment_container[fragment];
        PythonCodeContainer* cc=p.first;
        if (cc != NULL){
            int fragment_bloc_number=p.second;
            if (arrow_pixmap==NULL){
                arrow_pixmap=new QPixmap(":/Icons/debug_arrow.png");
            }
            reset_debug_trace();
            cc->debug_arrow=fragment_bloc_number+(line-1);
            debug_arrow_container=cc;
            QTextBlock block=cc->document()->findBlockByNumber(cc->debug_arrow);
            QTextCursor prev_cursor=cc->textCursor();
            QTextCursor cursor(prev_cursor);
            cursor.setPosition(block.position());
            cc->setTextCursor(cursor);
            ensureCursorVisible();
            cc->setTextCursor(prev_cursor);
            block.setVisible(false);
            block.setVisible(true);
        }
    }
}

void PythonCodeContainer::reset_debug_trace(){
        if (debug_arrow_container){
            QTextBlock pb=debug_arrow_container->document()->findBlockByNumber(debug_arrow_container->debug_arrow);
            debug_arrow_container->debug_arrow=-1;
            debug_arrow_container=NULL;
            pb.setVisible(false);
            pb.setVisible(true);//update line
        }
}

void PythonCodeContainer::repaint_border_area(QPaintEvent *event){
    QPainter painter(border_area);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int vertical_displace=tool_bar->height();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top()+vertical_displace;
    int bottom = top + (int) blockBoundingRect(block).height();
    painter.setPen(Qt::black);
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            switch (block.userState()){
            case LINE_NUMBER://display bloc number
                painter.drawText(0, top, border_width-7, fontMetrics().height(),
                                 Qt::AlignRight, QString::number(blockNumber + 1));
                break;
            case PROMPT_1://display prompt 1
                painter.drawText(0, top, border_width-7, fontMetrics().height(),
                                 Qt::AlignRight, ">>>");
                break;
            case PROMPT_2://display prompt 2
                painter.drawText(0, top, border_width-7, fontMetrics().height(),
                                 Qt::AlignRight, "...");
                break;
            default:
                painter.drawText(0, top, border_width-7, fontMetrics().height(),
                                 Qt::AlignRight, QString::number(block.userState()));
            }
            if (blockNumber == debug_arrow){
                painter.drawPixmap(border_width-6,top+(fontMetrics().height()-10),5,10,*arrow_pixmap);
            }
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        blockNumber++;
    }
}

void PythonCodeContainer::resizeEvent(QResizeEvent *e){
    QPlainTextEdit::resizeEvent(e);
    QRect cr=contentsRect();
    border_area->setGeometry(cr.left(),cr.top(),border_width,cr.bottom());
}

void PythonCodeContainer::keyPressEvent(QKeyEvent *e){
    static QRegExp indent_case("^[^(#|:)]*:");
    QTextCursor c=textCursor();
    if (completer != NULL && completer->popup() != NULL && completer->popup()->isVisible()){
        switch(e->key()){
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
        QString word_under_cursor=get_word_under_cursor();
        QAbstractItemView* popup=completer->popup();
        if (e->text().size()>0){//text entered
            completer->setCompletionPrefix(word_under_cursor);
            popup->setCurrentIndex(completer->completionModel()->index(0,0));
            QRect completer_rect=cursorRect();
            completer_rect.setWidth(popup->sizeHintForColumn(0)
                                    +popup->verticalScrollBar()->sizeHint().width());
            completer->complete(completer_rect);
        }
        if (word_under_cursor.length() < 1)
            popup->hide();

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
            c.movePosition(QTextCursor::EndOfBlock);
            c.insertText("\n");
            if (tab_number>0){
                for (int i=0;i<tab_number;i++)
                    c.insertText("\t");
            }
            setTextCursor(c);
            new_line(tab_number);
        }else if (e->modifiers()==Qt::ControlModifier && e->key()==Qt::Key_Space){
            QString word_under_cursor=get_word_under_cursor();
            if (word_under_cursor.length() >= 1){
                c.movePosition(QTextCursor::EndOfWord);
                setTextCursor(c);
                if (static_cast<PythonCodeContainer*>(completer->widget()) != NULL)
                    disconnect(completer,SIGNAL(activated(QString)),static_cast<PythonCodeContainer*>(completer->widget()),SLOT(insert_completion(QString)));
                completer->setWidget(this);
                completer->setCompletionPrefix(word_under_cursor);
                QAbstractItemView* popup=completer->popup();
                popup->setCurrentIndex(completer->completionModel()->index(0,0));
                QRect completer_rect=cursorRect();
                completer_rect.moveTop(tool_bar->height());
                completer_rect.moveRight(border_width);
                completer_rect.setWidth(popup->sizeHintForColumn(0)
                                        +popup->verticalScrollBar()->sizeHint().width());
                completer->complete(completer_rect);
                connect(completer,SIGNAL(activated(QString)),this,SLOT(insert_completion(QString)));
            }
        }else{
            key_press_event(e);
        }
    }
}

void PythonCodeContainer::insert_completion(QString completion){
    QTextCursor cursor=textCursor();
    cursor.movePosition(QTextCursor::Left);
    cursor.movePosition(QTextCursor::EndOfWord);
    cursor.insertText(completion.right(completion.length()
                                       -completer->completionPrefix().length()));
    setTextCursor(cursor);
}

void PythonCodeContainer::keywords_changed(const QStringList &keywords){
    completer->setModel(new QStringListModel(keywords));
}

QString PythonCodeContainer::get_word_under_cursor(){
    QTextCursor c=textCursor();
    QString block=c.block().text();
    static QRegExp complete_word("[\\w\\.]+");
    int position_in_block=c.positionInBlock();
    int index = complete_word.indexIn(block);
    while (index >= 0) {
        int length = complete_word.matchedLength();
        if (index < position_in_block && index+length >= position_in_block)
            return block.mid(index,length);
        index = complete_word.indexIn(block, index + length);
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
