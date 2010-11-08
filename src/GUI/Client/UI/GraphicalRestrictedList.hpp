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

    GraphicalRestrictedList(CF::Common::Option::ConstPtr opt = CF::Common::Option::ConstPtr(),
                            QWidget * parent = 0);

    ~GraphicalRestrictedList();

    void setRestrictedList(const QStringList & list);

    virtual bool setValue(const QVariant & value);

    virtual QVariant getValue() const;

  private slots:

    void currentIndexChanged(int);

  private:

    QComboBox * m_comboChoices;

    template<typename TYPE>
    void vectToStringList(const std::vector<boost::any> & vect,
                          QStringList & list) const
    {
      std::vector<boost::any>::const_iterator it = vect.begin();

      for( ; it != vect.end() ; it++)
        list << CF::Common::from_value( boost::any_cast<TYPE>(*it) ).c_str();
    }
  }; // class GraphicalRestrictedList

  /////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_GaphicalRestrictedList_hpp
