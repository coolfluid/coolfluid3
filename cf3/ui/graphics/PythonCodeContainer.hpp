// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_ui_graphics_PythonCodeContainer_hpp
#define cf3_ui_graphics_PythonCodeContainer_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QWidget>
#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QStringList>
#include <QCompleter>
#include <QHash>
#include <QPainter>

#include "ui/graphics/LibGraphics.hpp"
#include "ui/graphics/PythonSyntaxeHighlighter.hpp"

////////////////////////////////////////////////////////////////////////////////

class QToolBar;

////////////////////////////////////////////////////////////////////////////////


namespace cf3 {
namespace ui {
namespace graphics {

class BorderArea;
class DebugArrow;

class Graphics_API PythonCodeContainer : public QPlainTextEdit
{
    Q_OBJECT
public:
    enum line_type{
        LINE_NUMBER = -1,
        PROMPT_1 = 10000,
        PROMPT_2 = 10001
    };

    PythonCodeContainer(QWidget *parent = 0);
    ~PythonCodeContainer();
    void register_fragment(QString code,int block_number);
    void remove_fragments();
    virtual void key_press_event(QKeyEvent *e) = 0;
    virtual void new_line(int indent_number){}
    void repaint_border_area(QPaintEvent *event);
protected:
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
protected slots:
    void update_border_area(const QRect &,int);
    void keywords_changed(const QStringList &keywords);
    void insert_completion(QString);
    void display_debug_trace(int fragment,int line);
    void reset_debug_trace();
protected:
    QToolBar *tool_bar;
private:
    QString get_word_under_cursor();
    PythonSyntaxeHighlighter* highlighter;
    BorderArea *border_area;
    int border_width;
    static QMap<int,QPair<PythonCodeContainer*,int> > fragment_container;
    static int fragment_generator;
    static QCompleter *completer;
    static QPixmap *arrow_pixmap;
    static PythonCodeContainer *debug_arrow_container;
    int debug_arrow;//block number of the debug arrow, -1 for no arrow
};

////////////////////////////////////////////////////////////////////////////////

class BorderArea : public QWidget
 {
 public:
    BorderArea(PythonCodeContainer *container,int width)
        : QWidget(container) , container(container) , width(width) {}

    static QPixmap* arrow_pixmap;
 protected:
     void paintEvent(QPaintEvent *event) {
         container->repaint_border_area(event);
     }

 private:
     PythonCodeContainer *container;
     int width;
 };

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonCodeContainer_hpp
