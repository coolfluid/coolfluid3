// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_GraphicalArrayRestrictedList_hpp
#define cf3_GUI_Graphics_GraphicalArrayRestrictedList_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/Option.hpp"
#include "UI/Graphics/GraphicalValue.hpp"

class QGridLayout;
class QGroupBox;
class QListView;
class QPushButton;
class QStringListModel;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Graphics {

  //////////////////////////////////////////////////////////////////////////

  class GraphicalArrayRestrictedList :
      public GraphicalValue
  {
    Q_OBJECT
  public:

    GraphicalArrayRestrictedList(cf3::common::Option::ConstPtr opt = cf3::common::Option::ConstPtr(),
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

    QGroupBox * m_groupBox;

    QGridLayout * m_boxLayout;

    QVBoxLayout * m_buttonsLayout;

    QPushButton * m_btAdd;

    QPushButton * m_btRemove;

    template<typename TYPE>
    void vectToStringList(const std::vector<boost::any> & vect,
                          QStringList & list) const;

    template<typename TYPE>
    void anyToStringList(const boost::any & value, QStringList & list) const;

  }; // class GraphicalArrayRestrictedList

  //////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Graphics_GraphicalArrayRestrictedList_hpp
