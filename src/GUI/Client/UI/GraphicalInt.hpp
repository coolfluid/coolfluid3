// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_GraphicalInt_hpp
#define CF_GUI_Client_UI_GraphicalInt_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/GraphicalValue.hpp"

class QSpinBox;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  //////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalInt : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalInt(bool isUint, CF::Common::Option::ConstPtr opt = CF::Common::Option::ConstPtr(),
                 QWidget * parent = 0);

    ~GraphicalInt();

    virtual bool setValue(const QVariant & value);

    virtual QVariant value() const;

  private slots:

    void integerChanged(int value);

  private:

    QSpinBox * m_spinBox;

    bool m_isUint;

  }; // class GraphicalInt

  //////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_GraphicalInt_hpp
