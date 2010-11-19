// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_GraphicalUriArray_hpp
#define CF_GUI_Client_UI_GraphicalUriArray_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/GraphicalValue.hpp"

class QComboBox;
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


  class GraphicalUriArray : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUriArray(QWidget * parent = nullptr);

    ~GraphicalUriArray();

    void setProtocols(const std::vector<std::string> & protocols);

    virtual bool setValue(const QVariant & path);

    virtual QVariant value() const;

  protected:

    void keyPressEvent(QKeyEvent * event);

  private slots:

    void btBrowseClicked();

    void btAddClicked();

    void btRemoveClicked();

    void changeType(const QString & type);

  private:

    QLineEdit * m_editAdd;

    QStringListModel * m_model;

    QListView * m_listView;

    QPushButton * m_btAdd;

    QPushButton * m_btRemove;

    QVBoxLayout * m_buttonsLayout;

    QVBoxLayout * m_leftLayout;

    QComboBox * m_comboType;

    QHBoxLayout * m_topLayout;

    QPushButton * m_btBrowse;

  }; // class GraphicalArray

  //////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_GraphicalUriArray_hpp
