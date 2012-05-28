// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_ui_graphics_PythonCodeEditor_hpp
#define cf3_ui_graphics_PythonCodeEditor_hpp

////////////////////////////////////////////////////////////////////////////////

#include "ui/graphics/PythonCodeContainer.hpp"

#include "ui/graphics/LibGraphics.hpp"

////////////////////////////////////////////////////////////////////////////////

class QWidget;
class QKeyEvent;

////////////////////////////////////////////////////////////////////////////////


namespace cf3 {
namespace ui {
namespace graphics {

/// @brief simple python editor
class Graphics_API PythonCodeEditor: public PythonCodeContainer
{
  Q_OBJECT
public:
  PythonCodeEditor(QWidget *parent = 0);
protected:
  void key_press_event(QKeyEvent *);
  void new_line(int indent_number);
  void border_click(const QPoint &pos);
  bool editable_zone(const QTextCursor &cursor);
  void insert_text(const QString &text);
private slots:
  /// @brief send the code to the python console, the entire code is executed directly
  void execute_immediat();
  /// @brief send the code to the python console, the console stop after each statement to let the user do some modifications
  void execute_stepped();
  /// @brief open a file and append his content to the code editor
  void open();
  /// @brief save the code editor content to a file
  void save();
  /// @brief called at each modification, the main purpose of this is to change the new block state to LINE_NUMBER
  void contents_change(int pos,int sub,int add);
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonCodeEditor_hpp
