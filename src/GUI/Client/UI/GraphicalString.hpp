// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_ClientString_hpp
#define CF_GUI_Client_ClientString_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/GraphicalValue.hpp"

class QLineEdit;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalString : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalString(QWidget * parent = 0);

    ~GraphicalString();

    virtual bool setValue(const QVariant & value);

    virtual QVariant getValue() const;

  private slots:

    void textUpdated(const QString & text);

  private:

    QLineEdit * m_lineEdit;

  }; // class GraphicalDouble

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientString_hpp
