// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_GraphicalUrl_hpp
#define CF_GUI_Client_GraphicalUrl_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/GraphicalValue.hpp"

class QCompleter;
class QLineEdit;
class QPushButton;
class QString;
class QStringListModel;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////

  class ClientUI_API GraphicalUrl : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUrl(QWidget *parent = 0);

    ~GraphicalUrl();

    virtual bool setValue(const QVariant & path);

    virtual QVariant getValue() const;

  private slots:

    void btBrowseClicked();

    void updateModel(const QString & path);

  private:

    QLineEdit * m_editPath;

    QPushButton * m_btBrowse;

    QCompleter * m_completer;

    QStringListModel * m_completerModel;

  }; // class GraphicalUrl

////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_GraphicalUrl_hpp
