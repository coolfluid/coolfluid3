// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GraphicalInt_hpp
#define cf3_ui_Graphics_GraphicalInt_hpp

////////////////////////////////////////////////////////////////////////////

#include "ui/graphics/GraphicalValue.hpp"

class QDoubleSpinBox;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

class Graphics_API GraphicalInt : public GraphicalValue
{
  Q_OBJECT

public:

  GraphicalInt(bool isUint, QVariant value = 0, QWidget * parent = 0);

  ~GraphicalInt();

  virtual bool set_value(const QVariant & value);

  virtual QVariant value() const;

private slots:

  void integer_changed(double value);

private:

  QDoubleSpinBox * m_spin_box;

  bool m_isUint;

}; // class GraphicalInt

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GraphicalInt_hpp
