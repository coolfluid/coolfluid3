// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "PythonCodeEditor.hpp"
#include "ui/core/NScriptEngine.hpp"
#include "ui/core/NLog.hpp"
#include "common/Log.hpp"
#include <QStringListModel>
#include <QMessageBox>
#include <QTextBlock>
#include <QScrollBar>
#include <QToolBar>
#include <QAction>
#include <QWidget>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////


PythonCodeEditor::PythonCodeEditor(QWidget *parent) :
    PythonCodeContainer(parent)
{
    setUndoRedoEnabled(true);
    //Toolbar
    connect(tool_bar->addAction("Execute"),SIGNAL(triggered()),this,SLOT(execute()));
    connect(tool_bar->addAction("Open"),SIGNAL(triggered()),this,SLOT(open()));
    connect(tool_bar->addAction("Save"),SIGNAL(triggered()),this,SLOT(save()));
}

PythonCodeEditor::~PythonCodeEditor(){

}

void PythonCodeEditor::key_press_event(QKeyEvent *e){
    QTextCursor c=textCursor();
    switch(e->key()){
    case Qt::Key_Home:
        c.movePosition(QTextCursor::StartOfBlock);
        c.movePosition(QTextCursor::WordRight);
        setTextCursor(c);
        break;
    default:
        QPlainTextEdit::keyPressEvent(e);
    }
}

void PythonCodeEditor::new_line(int indent_number){

}

void PythonCodeEditor::execute(){
    static QRegExp two_point(":[\\s]*($|#[^$]*$)");
    remove_fragments();
    QTextBlock b=document()->firstBlock();
    while (b.isValid()){
        QString line=b.text();
        if (line.contains())
        //algorithme
        //find ':'
        b=b.next();
    }
}

void PythonCodeEditor::open(){
    remove_fragments();
}

void PythonCodeEditor::save(){
    remove_fragments();
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
