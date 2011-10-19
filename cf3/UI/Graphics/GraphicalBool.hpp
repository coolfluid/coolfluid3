// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_GraphicalBool_hpp
#define cf3_GUI_Graphics_GraphicalBool_hpp

////////////////////////////////////////////////////////////////////////////

#include "UI/Graphics/GraphicalValue.hpp"

class QCheckBox;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

  class Graphics_API GraphicalBool : public GraphicalValue
  {
    Q_OBJECT
  public:

    GraphicalBool(bool value = false, QWidget * parent = 0);

    ~GraphicalBool();

    virtual bool setValue(const QVariant & value);

    virtual QVariant value() const;

  private slots:

    void stateChanged(int state);

  private:

      QCheckBox * m_checkBox;

      void initGui();

  }; // class GraphicalBool

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_GraphicalBool_hpp
