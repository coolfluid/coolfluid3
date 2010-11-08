// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_GraphicalArray_hpp
#define CF_GUI_Client_UI_GraphicalArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/GraphicalValue.hpp"

class QKeyEvent;
class QLineEdit;
class QListView;
class QPushButton;
class QStringListModel;
class QValidator;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  //////////////////////////////////////////////////////////////////////////


  class GraphicalArray : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalArray(QValidator * validator = nullptr, QWidget * parent = nullptr);

    ~GraphicalArray();

    void setValidator(QValidator * validator);

    virtual bool setValue(const QVariant & path);

    virtual QVariant getValue() const;

  protected:

    void keyPressEvent(QKeyEvent * event);

  private slots:

    void btAddClicked();

    void btRemoveClicked();

  private:

    QLineEdit * m_editAdd;

    QStringListModel * m_model;

    QListView * m_listView;

    QPushButton * m_btAdd;

    QPushButton * m_btRemove;

    QVBoxLayout * m_buttonsLayout;

    QVBoxLayout * m_leftLayout;

  }; // class GraphicalArray

  //////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_GraphicalArray_hpp
