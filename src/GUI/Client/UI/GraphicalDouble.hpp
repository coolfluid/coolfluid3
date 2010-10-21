// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_ClientDouble_hpp
#define CF_GUI_Client_UI_ClientDouble_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/GraphicalValue.hpp"

class QDoubleValidator;
class QLineEdit;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  //////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalDouble : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalDouble(CF::Common::Option::ConstPtr opt = CF::Common::Option::ConstPtr(),
                    QWidget * parent = 0);

    ~GraphicalDouble();

    virtual bool setValue(const QVariant & value);

    virtual QVariant getValue() const;

  private slots:

    void textUpdated(const QString & text);

  private:

    QLineEdit * m_lineEdit;

    QDoubleValidator * m_validator;

    void initGui();

  }; // class GraphicalDouble

  //////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_ClientDouble_hpp
