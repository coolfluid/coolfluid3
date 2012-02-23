// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "PythonConsole.hpp"
#include "ui/core/NScriptEngine.hpp"
#include "common/Log.hpp"
#include <QStringListModel>
#include <QMessageBox>
#include <QTextBlock>
#include <QScrollBar>
////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////


PythonConsole::PythonConsole(QWidget *parent) :
    QTextEdit(parent)
{
    setLineWrapMode(QTextEdit::WidgetWidth);
    print_next_prompt();
    highlighter=new PythonSyntaxeHighlighter(document());
    history_index=0;
    completer_model=NULL;
    completer=new QCompleter(this);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseSensitive);
    completer->setWidget(this);
    //connect(&console_input)
    connect(this,SIGNAL(cursorPositionChanged()),this,SLOT(cursor_position_changed()));
    connect(completer,SIGNAL(activated(QString)),this,SLOT(insert_completion(QString)));
    connect(core::NScriptEngine::global().get(),SIGNAL(new_output(QString)),this,SLOT(insert_output(QString)));
    connect(core::NScriptEngine::global().get(),SIGNAL(complation_list_received(QStringList)),this,SLOT(keywords_changed(QStringList)));
}

PythonConsole::~PythonConsole(){

}

void PythonConsole::keyPressEvent(QKeyEvent *e){
    QTextCursor c=textCursor();
    QString command;
    if (completer->popup()->isVisible()){
        switch(e->key()){
        case Qt::Key_Enter:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
        case Qt::Key_Return:
            e->ignore();
            return;
        default:
            QTextEdit::keyPressEvent(e);
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
        if (e->modifiers()==Qt::ControlModifier && e->key()==Qt::Key_Space){
            QString word_under_cursor=get_word_under_cursor();
            if (word_under_cursor.length() >= 1){
                c.movePosition(QTextCursor::EndOfWord);
                setTextCursor(c);
                completer->setCompletionPrefix(word_under_cursor);
                QAbstractItemView* popup=completer->popup();
                popup->setCurrentIndex(completer->completionModel()->index(0,0));
                QRect completer_rect=cursorRect();
                completer_rect.setWidth(popup->sizeHintForColumn(0)
                                        +popup->verticalScrollBar()->sizeHint().width());
                completer->complete(completer_rect);
            }
        }else{
            switch(e->key()){
            case Qt::Key_Backspace:
            case Qt::Key_Left:
                if (c.positionInBlock() > prompt_position)
                    QTextEdit::keyPressEvent(e);
                break;
            case Qt::Key_Up:
                history_index--;
                if (history_index < 0)
                    history_index=0;
                document()->findBlockByNumber(input_block);
                c.movePosition(QTextCursor::StartOfBlock);
                c.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,prompt_position);
                c.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
                c.removeSelectedText();
                c.insertText(history.at(history_index));
                setTextCursor(c);
                break;
            case Qt::Key_Down:
                if (history_index < history.size()-1){
                    history_index++;
                    document()->findBlockByNumber(input_block);
                    c.movePosition(QTextCursor::StartOfBlock);
                    c.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,prompt_position);
                    c.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
                    c.removeSelectedText();
                    c.insertText(history.at(history_index));
                    setTextCursor(c);

                }
                break;
            case Qt::Key_Home:
                c.movePosition(QTextCursor::StartOfBlock);
                c.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,prompt_position);
                //lol
                ui::core::NScriptEngine::global()->get_complation_list();

                setTextCursor(c);
                break;
            case Qt::Key_Return:
                command=(document()->findBlockByNumber(input_block).text().mid(prompt_position));
                c.movePosition(QTextCursor::EndOfBlock);
                setTextCursor(c);
                ui::core::NScriptEngine::global()->execute_line(command);
                if (command.length()){
                    history.append(command);
                    history_index=history.size();
                }
                QTextEdit::keyPressEvent(e);
                print_next_prompt();
                break;
            default:
                QTextEdit::keyPressEvent(e);
            }
        }
    }
}

void PythonConsole::cursor_position_changed(){
    QTextCursor cursor=textCursor();
    setReadOnly(cursor.anchor() < prompt_start_in_text
            || cursor.position() < prompt_start_in_text);
}

void PythonConsole::insert_output(const QString &output){
    QTextCursor cursor=textCursor();
    cursor.setPosition(prompt_start_in_text);
    cursor.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,4);
    cursor.insertText(output+"\n");
    prompt_start_in_text+=output.size()+1;
    cursor.setPosition(prompt_start_in_text);
    input_block=cursor.blockNumber();
    ensureCursorVisible();
}

void PythonConsole::insert_completion(QString completion){
    QTextCursor cursor=textCursor();
    cursor.movePosition(QTextCursor::Left);
    cursor.movePosition(QTextCursor::EndOfWord);
    cursor.insertText(completion.right(completion.length()
                                       -completer->completionPrefix().length()));
    setTextCursor(cursor);
}

void PythonConsole::print_next_prompt(){
    setReadOnly(false);
    QTextCursor cursor=textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString(">>> "));
    input_block=cursor.blockNumber();
    prompt_position=cursor.columnNumber();
    prompt_start_in_text=cursor.position();
    setTextCursor(cursor);
    ensureCursorVisible();
}

void PythonConsole::keywords_changed(const QStringList &keywords){
    completer->setModel(new QStringListModel(keywords));
}

QString PythonConsole::get_word_under_cursor(){
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
