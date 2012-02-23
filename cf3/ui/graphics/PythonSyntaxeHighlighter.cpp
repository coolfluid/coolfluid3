// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "PythonSyntaxeHighlighter.hpp"
////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

PythonSyntaxeHighlighter::PythonSyntaxeHighlighter(QTextDocument* parent)
    :QSyntaxHighlighter(parent){
    HighlightingRule rule;
    QStringList keyword_list;
    keyword_list << "\\band\\b" << "\\belif\\b" << "\\bglobal\\b" << "\\bor\\b"
                 << "\\bassert\\b" << "\\belse\\b" << "\\bif\\b" << "\\bpass\\b"
                 << "\\bbreak\\b" << "\\bexcept\\b" << "\\bimport\\b" << "\\bprint\\b"
                 << "\\bclass\\b" << "\\bexec\\b" << "\\bin\\b" << "\\braise\\b"
                 << "\\bcontinue\\b" << "\\bfinally\\b" << "\\bis\\b" << "\\breturn\\b"
                 << "\\bdef\\b" << "\\bfor\\b" << "\\blambda\\b" << "\\btry\\b"
                 << "\\bdel\\b" << "\\bfrom\\b" << "\\bnot\\b" << "\\bwhile\\b";
    // keywords
    keyword_format.setForeground(QColor(0x20,0x4A,0x87));
    rule.pattern = QRegExp("\\b(and|elif|global|or|assert|else|if|pass|break|except|"
                               "import|print|class|exec|in|raise|continue|finally|is|return|"
                               "def|for|lambda|try|del|from|not|while)\\b");
    rule.format = keyword_format;
    highlighting_rules.append(rule);
    // comment
    comment_format.setForeground(QColor(0x8F,0x59,0x02));
    rule.pattern = QRegExp("#.*$");
    rule.format = comment_format;
    highlighting_rules.append(rule);

    // number
    number_format.setForeground(QColor(0x00,0x00,0xCF));
    rule.pattern = QRegExp("\\b\\d+\\b");
    rule.format = number_format;
    highlighting_rules.append(rule);

    // operator
    operator_format.setForeground(QColor(0xCE,0x5C,0x00));
    rule.pattern = QRegExp("\\+|\\-|\\*|\\/|\\=");
    rule.format = operator_format;
    highlighting_rules.append(rule);

    // string
    string_format.setForeground(QColor(0x4E,0x9A,0x06));
    rule.pattern = QRegExp("\"[^\"\\\\\\r\\n]*(?:\\\\.[^\"\\\\\\r\\n]*)*\"");
    rule.format = string_format;
    highlighting_rules.append(rule);

    // single quote string
    single_quote_string_format.setForeground(QColor(0x4E,0x9A,0x06));
    rule.pattern = QRegExp("\'[^\'\\\\\\r\\n]*(?:\\\\.[^\'\\\\\\r\\n]*)*\'");
    rule.format = single_quote_string_format;
    highlighting_rules.append(rule);

}

void PythonSyntaxeHighlighter::highlightBlock(const QString &text){
    foreach (const HighlightingRule &rule, highlighting_rules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
