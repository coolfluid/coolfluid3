// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GraphicalDouble_hpp
#define cf3_ui_Graphics_GraphicalDouble_hpp

////////////////////////////////////////////////////////////////////////////

#include "ui/graphics/GraphicalValue.hpp"

class QDoubleValidator;
class QLineEdit;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

class Graphics_API GraphicalDouble : public GraphicalValue
{
  Q_OBJECT

public:

  GraphicalDouble(Real value = 0.0, QWidget * parent = 0);

  ~GraphicalDouble();

  virtual bool set_value(const QVariant & value);

  virtual QVariant value() const;

private slots:

  void text_updated(const QString & text);

private:

  QLineEdit * m_line_edit;

  QDoubleValidator * m_validator;

}; // class GraphicalDouble

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GraphicalDouble_hpp
