// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "PythonConsole.hpp"
#include "ui/core/NScriptEngine.hpp"
#include "ui/core/NLog.hpp"
#include "ui/graphics/PythonConsole.hpp"
#include "common/Log.hpp"
#include <QStringListModel>
#include <QMessageBox>
#include <QTextBlock>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////


PythonConsole::PythonConsole(QWidget *parent) :
    PythonCodeContainer(parent)
{
    document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_1);
    history_index=0;
    input_start_in_text=0;
    output_line_number=1;
    input_block=0;

    setUndoRedoEnabled(false);

    //Toolbar
    QAction* line_by_line=new QAction("Line by line",this);
    line_by_line->setCheckable(true);
    tool_bar->addAction(line_by_line);
    stop_continue=new QAction("Stop",this);
    stopped=false;
    tool_bar->addAction(stop_continue);

    connect(line_by_line,SIGNAL(toggled(bool)),this,SLOT(line_by_line_activated(bool)));
    connect(stop_continue,SIGNAL(triggered()),this,SLOT(stop_continue_pressed()));
    connect(ui::core::NScriptEngine::global().get(),SIGNAL(debug_trace_received(int,int)),this,SLOT(execution_stopped()));

    connect(this,SIGNAL(cursorPositionChanged()),this,SLOT(cursor_position_changed()));
    connect(core::NScriptEngine::global().get(),SIGNAL(new_output(QString)),this,SLOT(insert_output(QString)));
    connect(core::NLog::global().get(), SIGNAL(new_message(QString, uiCommon::LogMessage::Type)),
            this, SLOT(insert_log(QString)));
}

PythonConsole::~PythonConsole(){

}

void PythonConsole::key_press_event(QKeyEvent *e){
    QTextCursor c=textCursor();
    QString command;
    switch(e->key()){
    case Qt::Key_Backspace:
    case Qt::Key_Left:
        if (c.positionInBlock() > 0){
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
            history_index--;
            if (history_index < 0)
                history_index=0;
            select_input(c);
            c.removeSelectedText();
            c.insertText(history.at(history_index));
            ensureCursorVisible();
            fix_prompt_history();
        }else{
            QPlainTextEdit::keyPressEvent(e);
        }
        break;
    case Qt::Key_Down:
        if (c.blockNumber() == document()->blockCount()-1){
            history_index++;
            select_input(c);
            c.removeSelectedText();
            if (history_index > history.size()-1)
                history_index=history.size()-1;
            else
                c.insertText(history.at(history_index));
            setTextCursor(c);
            ensureCursorVisible();
            fix_prompt_history();
        }else{
            QPlainTextEdit::keyPressEvent(e);
        }
        break;
    case Qt::Key_Home:
        c.movePosition(QTextCursor::StartOfBlock);
        c.movePosition(QTextCursor::WordRight);
        setTextCursor(c);
        select_input(c);
        QMessageBox::information(this,"test",c.selectedText());
        break;
    case Qt::Key_Return:
        select_input(c);
        command=c.selectedText();
        if (command.length()){
            if (e->modifiers() != Qt::ShiftModifier){
                document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_1);
            }else{
                document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_2);
            }
        }
        break;
    default:
        QPlainTextEdit::keyPressEvent(e);
    }
}

void PythonConsole::cursor_position_changed(){
    QTextCursor cursor=textCursor();
    setReadOnly(cursor.anchor() < input_start_in_text
            || cursor.position() < input_start_in_text);
}

void PythonConsole::fix_prompt_history(){
    QTextBlock block=document()->findBlockByNumber(input_block);
    block.setUserState(PythonCodeContainer::PROMPT_1);
    block=block.next();
    while (block.isValid()){
        block.setUserState(PythonCodeContainer::PROMPT_2);
        block=block.next();
    }
}


void PythonConsole::new_line(int indent_number){
    if (indent_number==0){//single line statement
        QTextCursor c=textCursor();
        execute_input(c);
        document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_1);
    }else{//multi line
        document()->lastBlock().setUserState(PythonCodeContainer::PROMPT_2);
    }

}

void PythonConsole::execute_input(QTextCursor &c){
    select_input(c);
    QString command=c.selectedText();
    register_fragment(command,input_block);
    command.chop(1);
    history.append(command);
    history_index=history.size();
    c.movePosition(QTextCursor::End);
    input_block=c.blockNumber();
    input_start_in_text=c.position();
    output_line_number=1;
    ensureCursorVisible();
}

void PythonConsole::execute_code(QString code,bool immediate){
    /*foreach (QString &line,code.split('\n')){

    }*/
}

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

void PythonConsole::insert_log(const QString &output){
    static QRegExp log_frame("\\[ [^\\]]* \\]\\[ [^\\]]* \\] ");
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
    insert_output(modified_output);
}

void PythonConsole::select_input(QTextCursor &cursor){
    cursor.setPosition(input_start_in_text);
    cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
}

void PythonConsole::line_by_line_activated(bool activated){
    if (activated)
        ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::LINE_BY_LINE_EXECUTION);
    else
        ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::NORMAL_EXECUTION);
}

void PythonConsole::stop_continue_pressed(){
    if (stopped){
        ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::CONTINUE);
        stop_continue->setText("Stop");
        reset_debug_trace();
    }else{
        ui::core::NScriptEngine::global().get()->emit_debug_command(ui::core::NScriptEngine::BREAK);
    }
    stop_continue->setEnabled(false);
}

void PythonConsole::execution_stopped(){
    stop_continue->setEnabled(true);
    stop_continue->setText("Continue");
    stopped=true;
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
