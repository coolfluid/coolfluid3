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
#include <QLabel>
#include <QToolTip>
#include <QTimer>

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
class PythonConsole;

class Graphics_API PythonCodeContainer : public QPlainTextEdit
{
  Q_OBJECT
public:
  enum line_type{
    LINE_NUMBER = -1,
    PROMPT_1 = 100000,
    PROMPT_2 = 100001
  };

  PythonCodeContainer(QWidget *parent = 0);
  ~PythonCodeContainer();
  void register_fragment(QString code,int block_number);
  void remove_fragments();
  virtual void key_press_event(QKeyEvent *e) = 0;
  virtual void new_line(int indent_number){}
  void repaint_border_area(QPaintEvent *event);
  QStringList generate_dictionary(QString prefix);
protected:
  void keyPressEvent(QKeyEvent *e);
  void resizeEvent(QResizeEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void leaveEvent(QEvent *);
protected slots:
  void update_border_area(const QRect &,int);
  void keywords_changed(const QStringList &add,const QStringList &sub);
  void insert_completion(QString);
  void display_debug_trace(int fragment,int line, const QStringList &scope_keys, const QStringList &scope_values);
  void reset_debug_trace();
  void request_documentation();
  void popup_documentation(const QString & documentation);
protected:
  QToolBar *tool_bar;
private:
  class PythonDict {
  public:
    PythonDict(){}
    PythonDict(const PythonDict &dict):name(dict.name),attribute(dict.attribute){}
    QString name;
    QVector<QString> attribute;
  };
  void add_to_dictionary(int &i,const QStringList &add,QString prefix);
  QString get_word_under_cursor();
  PythonSyntaxeHighlighter* highlighter;
  BorderArea *border_area;
  int border_width;
  int debug_arrow;//block number of the debug arrow, -1 for no arrow
  QPoint last_mouse_pos;
  QPoint offset_border;
  QTimer doc_timer;
  QString last_documented_word;
  QString last_documentation;
  static QMap<int,int> fragment_container;
  static int fragment_generator;
  static QCompleter *completer;
  static QPixmap *arrow_pixmap;
  static PythonCodeContainer *debug_arrow_container;
  static QVector<PythonDict> dictionary;
  static QStringList python_dictionary;
  static PythonConsole *python_console;
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

class PopupDocumentation : public QLabel
{
public:
  PopupDocumentation(PythonCodeContainer*parent,QString text)
    : QLabel(text,parent) {
    setWordWrap(true);
    setFixedWidth(400);
    setBackgroundRole(QPalette::ToolTipBase);
  }
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonCodeContainer_hpp
