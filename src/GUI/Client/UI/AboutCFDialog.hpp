// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_AboutCFDialog_hpp
#define CF_GUI_Client_UI_AboutCFDialog_hpp

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

  /// Builds and shows up the "About COOLFLuiD" dialog.
  /// This dialog displays some information about COOLFluiD, such as the kernel
  /// version, the operating system type,...
  /// @todo should display the license, CF logo, icons credits...
  class ClientUI_API AboutCFDialog : public QDialog
  {

  public:

    /// Constructor.
    /// @param parent The parent widget.
    AboutCFDialog(QWidget * parent = nullptr);

    /// Destructor.
    /// Frees all allocated memory.
    ~AboutCFDialog();

  private: // data

    /// Main layout
    QVBoxLayout * m_mainLayout;

    /// "OK" button
    QPushButton * m_btOK;

    /// Layout that contain the information.
    QFormLayout * m_infoLayout;

    /// Label for Qwt.
    QLabel * m_labQwt;

  }; // class AboutCFDialog

  ///////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_AboutCFDialo.hpp
