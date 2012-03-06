// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "PythonCodeEditor.hpp"
#include "PythonConsole.hpp"
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
#include <QFileDialog>

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
    connect(tool_bar->addAction("Execute all"),SIGNAL(triggered()),this,SLOT(execute_immediat()));
    connect(tool_bar->addAction("Execute statement by statement"),SIGNAL(triggered()),this,SLOT(execute_stepped()));
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

void PythonCodeEditor::execute_immediat(){
    PythonConsole::main_console->execute_code(toPlainText(),true);
}

void PythonCodeEditor::execute_stepped(){
    PythonConsole::main_console->execute_code(toPlainText(),false);
}

void PythonCodeEditor::open(){
    QFileDialog dlg;

  #ifndef Q_WS_MAC
    dlg.setOption(QFileDialog::DontUseNativeDialog);
  #endif

    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setNameFilters( QStringList() << "Python script (*.py)" << "All files (*.*)" );
    dlg.setDirectory( QDir::home() );
    dlg.setFileMode(QFileDialog::ExistingFile);
    if( dlg.exec() == QFileDialog::Accepted ){
      QFile f(dlg.selectedFiles().first());
      f.open(QFile::ReadOnly);
      clear();
      insertPlainText(f.readAll());
      f.close();
    }
}

void PythonCodeEditor::save(){
    QFileDialog dlg;

  #ifndef Q_WS_MAC
    dlg.setOption(QFileDialog::DontUseNativeDialog);
  #endif

    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilters( QStringList() << "Python script (*.py)" << "All files (*.*)" );
    dlg.setDirectory( QDir::home() );
    dlg.setFileMode(QFileDialog::AnyFile);

    if( dlg.exec() == QFileDialog::Accepted ){
      QFile f(dlg.selectedFiles().first());
      f.open(QFile::WriteOnly);
      f.write((const char*)toPlainText().constData());
      f.close();
    }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
