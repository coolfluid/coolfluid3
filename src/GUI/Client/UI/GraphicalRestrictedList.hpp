// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_GaphicalRestrictedList_hpp
#define CF_GUI_Client_UI_GaphicalRestrictedList_hpp

///////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/GraphicalValue.hpp"

class QComboBox;

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  /////////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalRestrictedList : public GraphicalValue
  {
    Q_OBJECT

  public:

      GraphicalRestrictedList(QWidget * parent = 0);

      ~GraphicalRestrictedList();

      virtual bool setValue(const QVariant & value);

      virtual QVariant getValue() const;

  private:

      QComboBox * m_comboChoices;

  private slots:



  }; // class GraphicalRestrictedList

  /////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_GaphicalRestrictedList_hpp
