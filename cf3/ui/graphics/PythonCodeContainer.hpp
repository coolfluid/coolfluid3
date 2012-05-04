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
#include <QStringList>
#include <QCompleter>
#include <QLabel>
#include <QTimer>
#include <QStandardItemModel>

#include "ui/graphics/LibGraphics.hpp"
#include "ui/graphics/PythonSyntaxeHighlighter.hpp"

////////////////////////////////////////////////////////////////////////////////

class QToolBar;
class QTreeView;
class QKeyEvent;

////////////////////////////////////////////////////////////////////////////////


namespace cf3 {
namespace ui {
namespace graphics {

class BorderArea;
class DebugArrow;
class PythonConsole;
class PythonCompleter;
class CustomStandardItem;


/// @brief This class contains common code for python editors
class Graphics_API PythonCodeContainer : public QPlainTextEdit
{
  Q_OBJECT
public:
  /// @brief Use to manage userData in a QTextBlock
  enum line_type{
    PROMPT_1 = 100000,
    PROMPT_2 = 100001,
    LINE_NUMBER = 100002
  };
  /// @brief constructor
  PythonCodeContainer(QWidget *parent = 0);

  /// @brief called by the border area on repaint event, display the line numbers from QTextBlock::userState
  void repaint_border_area(QPaintEvent *event);
  /// @brief called by the border area on click event
  virtual void border_click(const QPoint & pos) = 0;
protected: 
  /// @brief Send a toggle break command to the script engine
  void toggle_break_point(int fragment_block, int line_number,bool send=true);
  /// @brief manage some typing behaviour then call key_press_event to propagate the event to the inherited class
  void keyPressEvent(QKeyEvent *e);
  /// @brief repaint the border area
  void resizeEvent(QResizeEvent *e);
  /// @brief start a timer when the mouse is on a word, the timer will make a request_documentation for this word
  void mouseMoveEvent(QMouseEvent *e);
  /// @brief stop the documentation timer
  void leaveEvent(QEvent *);
  // @brief implement the drag feature REMOVED
  //void mousePressEvent(QMouseEvent *e);
  /// @brief create a mimeData with a preview of the selected text
  QMimeData* createMimeDataFromSelection() const;
  /// @brief allow to drop text mimeData and QListWidgetItem mimeData
  bool canInsertFromMimeData(const QMimeData *source) const;
  /// @brief call insert_text with the extracted mimeData
  void insertFromMimeData(const QMimeData *source);
  /// @brief call after the the common text
  virtual void key_press_event(QKeyEvent *e) = 0;
  /// @brief allow the console to known when execute a inputed code
  virtual void new_line(int indent_number){}
  /// @brief allow the python container to known if the position of the cursor is writable, used for the python console
  virtual bool editable_zone(const QTextCursor &cursor) = 0;
  /// @brief ask a text insertion to the inherited class
  virtual void insert_text(const QString & text) = 0;
protected slots:
  /// @brief connected to the scrollbar signal of the code zone, ask to the border area to do a repaint
  void update_border_area(const QRect &,int);
  void keywords_changed(const QStringList &add,const QStringList &sub);
  void insert_completion(QString);
  void request_documentation();
  void popup_documentation(const QString & documentation);
private:
  class PythonDict {
  public:
    PythonDict(){}
    PythonDict(const PythonDict &dict):name(dict.name),attribute(dict.attribute){}
    QString name;
    QVector<QString> attribute;
  };
  void add_to_dictionary(int &i,const QStringList &add,CustomStandardItem* item);
  void remove_dictionary_item(QString name,CustomStandardItem* item);
  QString get_word_under_cursor(QTextCursor &c);
  BorderArea *border_area;
  QPoint last_mouse_pos;
  QTimer doc_timer;
  QString last_documented_word;
  QString last_documentation;
  static PythonCompleter *completer;
  static QVector<PythonDict> dictionary;
  //static QStringList python_dictionary;
protected:
  int border_width;
  QPoint offset_border;
  QToolBar *tool_bar;
  static PythonConsole *python_console;
  static QStandardItemModel python_dictionary;
  static QTreeView *python_scope_values;
  QVector<int> break_points;
  PythonSyntaxeHighlighter* highlighter;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Used to display the the line number and the prompt on a PythonCodeContainer
class BorderArea : public QWidget
{
public:
  BorderArea(PythonCodeContainer *container,int width)
    : QWidget(container) , container(container) , width(width) {}
  static QPixmap* debug_arrow;
  static QPixmap* break_point;
protected:
  void paintEvent(QPaintEvent *event) {
    container->repaint_border_area(event);
  }

  void mousePressEvent(QMouseEvent *e){
    container->border_click(e->pos());
  }

private:
  PythonCodeContainer *container;
  int width;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Dervied QCompleter to allow the use of a tree model for completion model
class PythonCompleter : public QCompleter
{
public:
  PythonCompleter(PythonCodeContainer*parent)
    : QCompleter(parent) {
  }
 protected:
  QStringList splitPath(const QString &path) const{
    return path.split('.');
  }
  QString pathFromIndex(const QModelIndex &index) const{
    QStringList dataList;
    for (QModelIndex i = index; i.isValid(); i = i.parent()) {
      dataList.prepend(model()->data(i, completionRole()).toString());
    }
    return dataList.join(".");
  }
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Dervied QStandardItem to allow case insensitive sorting
class CustomStandardItem : public QStandardItem
{
public:
  CustomStandardItem(const QString &text) : QStandardItem(text) {}
  bool operator< ( const QStandardItem & other ) const{
    return text().toUpper() < other.text().toUpper();
  }
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonCodeContainer_hpp
