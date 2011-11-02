// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GraphicalUriArray_hpp
#define cf3_ui_Graphics_GraphicalUriArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "UI/Graphics/GraphicalValue.hpp"

class QComboBox;
class QKeyEvent;
class QGridLayout;
class QGroupBox;
class QItemSelection;
class QLineEdit;
class QListView;
class QPushButton;
class QStringListModel;
class QValidator;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

  //////////////////////////////////////////////////////////////////////////


  class GraphicalUriArray : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUriArray(const QString & sep = QString(), QWidget * parent = nullptr);

    ~GraphicalUriArray();

    void set_protocols(const std::vector<std::string> & protocols);

    virtual bool set_value(const QVariant & path);

    virtual QVariant value() const;

  private slots:

    void bt_add_clicked();

    void bt_remove_clicked();

    void scheme_changed(const QString & type);

    void move_up();

    void move_down();

    void selection_changed(const QItemSelection& selected, const QItemSelection & delected);

  private: // functions

    void move_items( int step );

  private:

    QLineEdit * m_edit_add;

    QStringListModel * m_model;

    QListView * m_list_view;

    QPushButton * m_bt_add;

    QPushButton * m_bt_remove;

    QPushButton * m_bt_up;

    QPushButton * m_bt_down;

    QVBoxLayout * m_buttons_layout;

    QComboBox * m_combo_type;

    QGridLayout * m_box_layout;

    QGroupBox * m_group_box;

  }; // class GraphicalArray

  //////////////////////////////////////////////////////////////////////////

} // namespace Graphics
} // namespace UI
} // namespace cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GraphicalUriArray_hpp
