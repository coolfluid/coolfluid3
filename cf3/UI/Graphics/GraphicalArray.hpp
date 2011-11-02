// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_GraphicalArray_hpp
#define cf3_ui_Graphics_GraphicalArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "UI/Graphics/GraphicalValue.hpp"

class QGroupBox;
class QGridLayout;
class QKeyEvent;
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


  class GraphicalArray : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalArray(QValidator * validator = nullptr, const QString& sep = QString(),
                   QWidget * parent = nullptr);

    ~GraphicalArray();

    void set_validator(QValidator * validator);

    virtual bool set_value(const QVariant & path);

    virtual QVariant value() const;

  protected:

    void keyPressEvent(QKeyEvent * event);

  private slots:

    void bt_remove_clicked();

  private:

    QLineEdit * m_edit_add;

    QStringListModel * m_model;

    QListView * m_list_view;

    QPushButton * m_bt_remove;

    QGridLayout * m_box_layout;

    QGroupBox * m_group_box;

  }; // class GraphicalArray

  //////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_GraphicalArray_hpp
