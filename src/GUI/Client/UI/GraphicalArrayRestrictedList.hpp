// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_GraphicalArrayRestrictedList_hpp
#define CF_GUI_Client_UI_GraphicalArrayRestrictedList_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/Option.hpp"
#include "GUI/Client/UI/GraphicalValue.hpp"

class QListView;
class QStandardItemModel;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  //////////////////////////////////////////////////////////////////////////

  class GraphicalArrayRestrictedList :
      public GraphicalValue
  {
    Q_OBJECT
  public:

    GraphicalArrayRestrictedList(CF::Common::Option::ConstPtr opt = CF::Common::Option::ConstPtr(),
                                 QWidget * parent = 0);

    ~GraphicalArrayRestrictedList();

    void setRestrictedList(const QStringList & list);

    virtual bool setValue(const QVariant & value);

    virtual QVariant value() const;

  private slots:

    // clicked checkbox

  private:

    QListView * m_listView;

    QStandardItemModel * m_model;

    template<typename TYPE>
    void vectToStringList(const std::vector<boost::any> & vect,
                          QStringList & list) const
    {
      std::vector<boost::any>::const_iterator it = vect.begin();

      try
      {
        for( ; it != vect.end() ; it++)
          list << CF::Common::from_value( boost::any_cast<TYPE>(*it) ).c_str();
      }
      catch(boost::bad_any_cast & bac)
      {
        throw CF::Common::CastingFailed(FromHere(), "Unable to cast [" +
                                        CF::Common::demangle(it->type().name())
                                        + "] to [" +
                                        std::string(CF::Common::XmlTag<TYPE>::type())+"]");
      }
    }
  }; // class GraphicalArrayRestrictedList

  //////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_GraphicalArrayRestrictedList_hpp
