// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_GraphicalArray_hpp
#define CF_GUI_Client_UI_GraphicalArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/NRemoteOpen.hpp"
#include "GUI/Client/UI/GraphicalValue.hpp"

class QListView;
class QPushButton;
class QStringListModel;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

  /////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalUrlArray : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUrlArray(QWidget * parent = 0);

    ~GraphicalUrlArray();

    virtual QVariant getValue() const;

    virtual bool setValue(const QVariant & value);

  private slots:

    void btAddClicked();

    void btRemoveClicked();

  private:

    QStringListModel * m_model;

    QListView * m_listView;

    QPushButton * m_btAdd;

    QPushButton * m_btRemove;

    QVBoxLayout * m_buttonsLayout;

    NRemoteOpen::Ptr m_browser;

  };

  /////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

///////////////////////////////////////////////////////////////////////////


#endif // CF_GUI_Client_UI_GraphicalArray_hpp
