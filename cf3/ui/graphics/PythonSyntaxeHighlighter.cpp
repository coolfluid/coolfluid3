// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "PythonSyntaxeHighlighter.hpp"
#include <QColor>

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

int PythonSyntaxeHighlighter::instance_count=0;
QVector<PythonSyntaxeHighlighter::HighlightingRule> *PythonSyntaxeHighlighter::highlighting_rules=0;
QTextCharFormat *PythonSyntaxeHighlighter::comment_format=0;
QTextCharFormat *PythonSyntaxeHighlighter::keyword_format=0;
QTextCharFormat *PythonSyntaxeHighlighter::number_format=0;
QTextCharFormat *PythonSyntaxeHighlighter::operator_format=0;
QTextCharFormat *PythonSyntaxeHighlighter::string_format=0;
QTextCharFormat *PythonSyntaxeHighlighter::single_quote_string_format=0;

//////////////////////////////////////////////////////////////////////////

PythonSyntaxeHighlighter::PythonSyntaxeHighlighter(QTextDocument* parent)
    :QSyntaxHighlighter(parent){
  if (!instance_count++){
    HighlightingRule rule;
    highlighting_rules=new QVector<HighlightingRule>();
    // keywords
    keyword_format=new QTextCharFormat();
    keyword_format->setForeground(QColor(0x20,0x4A,0x87));
    rule.pattern = QRegExp("\\b(and|elif|global|or|assert|else|if|pass|break|except|"
                               "import|print|class|exec|in|raise|continue|finally|is|return|"
                               "def|for|lambda|try|del|from|not|while|True|False)\\b");
    rule.format = keyword_format;
    highlighting_rules->append(rule);
    // comment
    comment_format=new QTextCharFormat();
    comment_format->setForeground(QColor(0x8F,0x59,0x02));
    rule.pattern = QRegExp("#[^\\r\\n]*");
    rule.format = comment_format;
    highlighting_rules->append(rule);

    // number
    number_format=new QTextCharFormat();
    number_format->setForeground(QColor(0x00,0x00,0xCF));
    rule.pattern = QRegExp("\\b\\d+\\b");
    rule.format = number_format;
    highlighting_rules->append(rule);

    // operator
    operator_format=new QTextCharFormat();
    operator_format->setForeground(QColor(0xCE,0x5C,0x00));
    rule.pattern = QRegExp("\\+|\\-|\\*|\\/|\\=");
    rule.format = operator_format;
    highlighting_rules->append(rule);

    // string
    string_format=new QTextCharFormat();
    string_format->setForeground(QColor(0x4E,0x9A,0x06));
    rule.pattern = QRegExp("\"[^\"\\\\\\r\\n]*(?:\\\\.[^\"\\\\\\r\\n]*)*\"");
    rule.format = string_format;
    highlighting_rules->append(rule);

    // single quote string
    single_quote_string_format=new QTextCharFormat();
    single_quote_string_format->setForeground(QColor(0x4E,0x9A,0x06));
    rule.pattern = QRegExp("\'[^\'\\\\\\r\\n]*(?:\\\\.[^\'\\\\\\r\\n]*)*\'");
    rule.format = single_quote_string_format;
    highlighting_rules->append(rule);
  }
}

PythonSyntaxeHighlighter::~PythonSyntaxeHighlighter(){
  if (!--instance_count){
    delete highlighting_rules;
    delete comment_format;
    delete keyword_format;
    delete number_format;
    delete operator_format;
    delete string_format;
    delete single_quote_string_format;
  }
}

void PythonSyntaxeHighlighter::highlightBlock(const QString &text){
  foreach (const HighlightingRule &rule, *highlighting_rules) {
    QRegExp expression(rule.pattern);
    int index = expression.indexIn(text);
    while (index >= 0) {
      int length = expression.matchedLength();
      setFormat(index, length, *rule.format);
      index = expression.indexIn(text, index + length);
    }
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
