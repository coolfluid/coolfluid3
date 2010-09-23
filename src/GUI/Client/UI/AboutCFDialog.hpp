// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_AboutCFDialog_hpp
#define CF_GUI_Client_AboutCFDialog_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QDialog>
#include <QList>

#include "GUI/Client/UI/LibClientUI.hpp"

class QFormLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  class ClientUI_API AboutCFDialog : public QDialog
  {
    struct CFInfo
    {
      public:
      QLabel * labName;
      QLabel * labValue;

      CFInfo(const QString & name, const QString & value, QFormLayout * parent);

      ~CFInfo();
    };

  public:

    AboutCFDialog(QWidget * parent = CFNULL);

    ~AboutCFDialog();

  private: // data

    QVBoxLayout * m_mainLayout;

    QPushButton * m_btOK;

    QFormLayout * m_infoLayout;

    QList<CFInfo *> m_infoList;

  }; // class AboutCFDialog

  ///////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_AboutCFDialo.hpp
