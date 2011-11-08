// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GraphicalArrayRestrictedList_hpp
#define cf3_ui_Graphics_GraphicalArrayRestrictedList_hpp

////////////////////////////////////////////////////////////////////////////

#include "common/Option.hpp"
#include "ui/graphics/GraphicalValue.hpp"

class QGridLayout;
class QGroupBox;
class QListView;
class QPushButton;
class QStringListModel;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

  //////////////////////////////////////////////////////////////////////////

  class GraphicalArrayRestrictedList :
      public GraphicalValue
  {
    Q_OBJECT
  public:

    GraphicalArrayRestrictedList( common::Option::ConstPtr opt = common::Option::ConstPtr(),
                                  QWidget * parent = 0 );

    ~GraphicalArrayRestrictedList();

    void set_restricted_list(const QStringList & list);

    virtual bool set_value(const QVariant & value);

    virtual QVariant value() const;

  private slots:

    void bt_add_clicked();

    void bt_remove_clicked();

  private:

    QListView * m_allowed_list_view;

    QListView * m_selected_list_view;

    QStringListModel * m_allowed_model;

    QStringListModel * m_selected_model;

    QGroupBox * m_group_box;

    QGridLayout * m_box_layout;

    QVBoxLayout * m_buttons_layout;

    QPushButton * m_bt_add;

    QPushButton * m_bt_remove;

    template<typename TYPE>
    void vect_to_stringlist( const std::vector<boost::any> & vect,
                             QStringList & list ) const;

    template<typename TYPE>
    void any_to_stringlist( const boost::any & value, QStringList & list ) const;

  }; // class GraphicalArrayRestrictedList

  //////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GraphicalArrayRestrictedList_hpp
