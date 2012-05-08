// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "PythonSyntaxeHighlighter.hpp"
#include "PythonPreCompiler.hpp"
#include "PythonCodeContainer.hpp"
#include <QColor>


////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

int PythonSyntaxeHighlighter::instance_count=0;
QVector<PythonSyntaxeHighlighter::HighlightingRule> *PythonSyntaxeHighlighter::highlighting_rules=NULL;
QTextCharFormat *PythonSyntaxeHighlighter::comment_format=NULL;
QTextCharFormat *PythonSyntaxeHighlighter::keyword_format=NULL;
QTextCharFormat *PythonSyntaxeHighlighter::number_format=NULL;
QTextCharFormat *PythonSyntaxeHighlighter::operator_format=NULL;
QTextCharFormat *PythonSyntaxeHighlighter::string_format=NULL;
QTextCharFormat *PythonSyntaxeHighlighter::single_quote_string_format=NULL;
QTextCharFormat *PythonSyntaxeHighlighter::error_format=NULL;
PythonPreCompiler *PythonSyntaxeHighlighter::python_pre_compiler=NULL;

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

    //error displaying
    error_format=new QTextCharFormat();
    error_format->setUnderlineColor(QColor(Qt::red));
    error_format->setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    python_pre_compiler=new PythonPreCompiler();
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
  static QRegExp two_points("^[^#:]*:[ ]*(#[^$]*)?$");
  currentBlock().setUserData(NULL);//reset error if there where one
  if (text.length()){
    int a=currentBlock().userState();
    if (a >= PythonCodeContainer::PROMPT_1){
      int start=0,end=text.size();
      int first_word=0;
      if (text.contains(two_points)){
        first_word=1;
        for (;start<text.size();start++){
          QChar c=text[start];
          if (c != '#'){
            if (first_word==1){
              if (c != '\t' && c != ' ')
                first_word=2;
            }else if (first_word==2){
              if (c == '\t' || c == ' ')
                first_word=3;
            }else{
              if (c != '\t' && c != ' '){
                first_word=4;
                break;
              }
            }
          }
        }
      }else{
        for (;start<end && (text[start] == '\t' || text[start] == ' ');start++);
      }
      end=start;
      for (;end<text.size() && text[end] != ':' && text[end] != '#';end++);
      if ((first_word == 0 || first_word==4) && start != end){
        const QString error=python_pre_compiler->try_compile(text.mid(start,end-start));
        if (error.length()){
          setFormat(start,end-start,*error_format);
          currentBlock().setUserData(new TextBlockErrorData(error));
          return;
        }
      }
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
  }
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
