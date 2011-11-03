// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GraphicalBool_hpp
#define cf3_ui_Graphics_GraphicalBool_hpp

////////////////////////////////////////////////////////////////////////////

#include "ui/graphics/GraphicalValue.hpp"

class QCheckBox;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalBool : public GraphicalValue
  {
    Q_OBJECT
  public:

    GraphicalBool(bool value = false, QWidget * parent = 0);

    ~GraphicalBool();

    virtual bool set_value(const QVariant & value);

    virtual QVariant value() const;

  private slots:

    void state_changed(int state);

  private:

      QCheckBox * m_check_box;

  }; // class GraphicalBool

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GraphicalBool_hpp
