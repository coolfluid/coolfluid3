// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_ui_graphics_PythonSyntaxeHighlighter_hpp
#define cf3_ui_graphics_PythonSyntaxeHighlighter_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QSyntaxHighlighter>

#include "ui/graphics/LibGraphics.hpp"

#include <QTextBlockUserData>

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

class PythonPreCompiler;

/// @brief simple syntaxe highlighter
class Graphics_API PythonSyntaxeHighlighter
        : public QSyntaxHighlighter
{
    Q_OBJECT
public:
  /// @brief allocate the syntaxe rules on first init
  PythonSyntaxeHighlighter(QTextDocument* parent);
  /// @brief free the syntaxe rules on last delete
  ~PythonSyntaxeHighlighter();
protected:
  void highlightBlock(const QString &text);
private:
  struct HighlightingRule
  {
      QRegExp pattern;
      QTextCharFormat *format;
  };
  static int instance_count;
  static QVector<HighlightingRule> *highlighting_rules;
  static QTextCharFormat *comment_format;
  static QTextCharFormat *keyword_format;
  static QTextCharFormat *number_format;
  static QTextCharFormat *operator_format;
  static QTextCharFormat *string_format;
  static QTextCharFormat *single_quote_string_format;
  static QTextCharFormat *error_format;
  static PythonPreCompiler *python_pre_compiler;
}; // PythonSyntaxeHighlighter

////////////////////////////////////////////////////////////////////////////////

class TextBlockErrorData
  : public QTextBlockUserData {
public:
  TextBlockErrorData(const QString & str) : error_text(str) {}
  const QString & get_error_string() {return error_text;}
private:
  const QString error_text;
};

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonSyntaxeHighlighter_hpp
