// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_ClientBool_hpp
#define CF_GUI_Client_UI_ClientBool_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/GraphicalValue.hpp"

class QCheckBox;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  //////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalBool : public GraphicalValue
  {
    Q_OBJECT
  public:

    GraphicalBool(QWidget * parent = 0);

    ~GraphicalBool();

    virtual bool setValue(const QVariant & value);

    virtual QVariant getValue() const;

  private slots:

    void stateChanged(int state);

  private:

      QCheckBox * m_checkBox;

  }; // class GraphicalBool

  //////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_ClientBool_hpp
