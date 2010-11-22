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
class QPushButton;
class QStringListModel;
class QVBoxLayout;

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

    void btAddClicked();

    void btRemoveClicked();

  private:

    QListView * m_allowedListView;

    QListView * m_selectedListView;

    QStringListModel * m_allowedModel;

    QStringListModel * m_selectedModel;

    QVBoxLayout * m_buttonsLayout;

    QPushButton * m_btAdd;

    QPushButton * m_btRemove;

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
        std::string realType = CF::Common::demangle(it->type().name());
        std::string typeToCast = CF::Common::XmlTag<TYPE>::type();

        throw CF::Common::CastingFailed(FromHere(), "Unable to cast [" + realType
                                        + "] to [" + typeToCast +"]");
      }
    }

    template<typename TYPE>
    void anyToStringList(const boost::any & value, QStringList & list) const
    {
      try
      {
        std::vector<TYPE> vect = boost::any_cast<std::vector<TYPE> >(value);
        typename std::vector<TYPE>::const_iterator it = vect.begin();

        for( ; it != vect.end() ; it++)
          list << CF::Common::from_value(*it).c_str();
      }
      catch(boost::bad_any_cast & bac)
      {
        std::string realType = CF::Common::demangle(value.type().name());
        std::string typeToCast = CF::Common::XmlTag<TYPE>::type();

        throw CF::Common::CastingFailed(FromHere(), "Unable to cast [" + realType
                                        + "] to [" + typeToCast +"]");
      }
    }
  }; // class GraphicalArrayRestrictedList

  //////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_GraphicalArrayRestrictedList_hpp
