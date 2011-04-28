// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//header
#include "UI/QwtTab/ColorSelector.hpp"

////////////////////////////////////////////////////////////////////////////////

class QColorDialog;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace QwtTab {

////////////////////////////////////////////////////////////////////////////////

ColorSelector::ColorSelector(int raw,QWidget * parent )
        :QLabel(parent)
{
    this->m_raw = raw;
    this->setText("color");
}


void ColorSelector::set_color()
{
  m_color = QColorDialog::getColor(Qt::red,this);
  if (m_color.isValid()) {
    QPalette pal(QColor(255,255,255));
    pal.setColor( QPalette::Text, m_color);
    this->setPalette(pal);
  }else{
    m_color = Qt::black;
    QPalette pal(QColor(255,255,255));
    pal.setColor( QPalette::Text, m_color);
    this->setPalette(pal);
  }
  QString labelTemp("color");
  labelTemp += "(";
  labelTemp += m_color.name();
  labelTemp += ")";
  this->setText(labelTemp);

  emit valueChanged(m_color,m_raw);
}


void ColorSelector::set_color(QColor color){
  m_color = color;
  if (m_color.isValid()) {
      QPalette pal(QColor(255,255,255));
      pal.setColor( QPalette::Text, m_color);
      this->setPalette(pal);
  }else{
      m_color = Qt::black;
      QPalette pal(QColor(255,255,255));
      pal.setColor( QPalette::Text, m_color);
      this->setPalette(pal);
  }
  QString labelTemp("color");
  labelTemp += "(";
  labelTemp += m_color.name();
  labelTemp += ")";
  this->setText(labelTemp);
}



void ColorSelector::mousePressEvent ( QMouseEvent * event )
{
    this->set_color();
}

QColor ColorSelector::get_color(){
    return m_color;
}

void ColorSelector::set_row(int row){
  this->m_raw = row;
}

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF
