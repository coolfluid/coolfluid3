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

#include "ui/graphics/LibGraphics.hpp"
#include "ui/graphics/PythonSyntaxeHighlighter.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

class Graphics_API PythonConsole : public QTextEdit
{
    Q_OBJECT
public:
    PythonConsole(QWidget *parent = 0);
    ~PythonConsole();
    
signals:

protected:
    void keyPressEvent(QKeyEvent *e);
private slots:
    void cursor_position_changed();
    void keywords_changed(const QStringList &keywords);
    void insert_completion(QString);
    void insert_output(const QString &output);
private:
    /// Start of prompt into the input block
    int prompt_position;
    /// Index of the block that contains the current prompt
    int input_block;

    int prompt_start_in_text;

    void print_next_prompt();

    QString get_word_under_cursor();

    PythonSyntaxeHighlighter* highlighter;
    QAbstractItemModel* completer_model;
    QCompleter* completer;
    QStringList history;
    int history_index;
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonConsole_hpp
