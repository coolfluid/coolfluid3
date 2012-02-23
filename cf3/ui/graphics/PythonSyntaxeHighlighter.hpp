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
#include <QColor>

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

class Graphics_API PythonSyntaxeHighlighter
        : public QSyntaxHighlighter
{
public:
    PythonSyntaxeHighlighter(QTextDocument* parent);
protected:
    void highlightBlock(const QString &text);
private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlighting_rules;
    QTextCharFormat comment_format;
    QTextCharFormat keyword_format;
    QTextCharFormat number_format;
    QTextCharFormat operator_format;
    QTextCharFormat string_format;
    QTextCharFormat single_quote_string_format;
}; // PythonSyntaxeHighlighter

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonSyntaxeHighlighter_hpp
