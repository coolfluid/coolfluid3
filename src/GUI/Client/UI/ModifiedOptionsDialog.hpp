// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_ModifiedOptionsDialog_hpp
#define CF_GUI_Client_UI_ModifiedOptionsDialog_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "Common/CF.hpp"

#include "GUI/Client/UI/LibClientUI.hpp"

class QDialogButtonBox;
class QPushButton;
class QTableView;
class QVBoxLayout;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

namespace ClientCore { class CommitDetails; }

namespace ClientUI {

////////////////////////////////////////////////////////////////////////////

/// @brief Dialog that shows commit details.
class ClientUI_API ModifiedOptionsDialog : public QDialog
{
  Q_OBJECT

public:

  /// @brief Constructor
  /// @param parent Parent widget. May be @c nullptr.
  ModifiedOptionsDialog(QWidget * parent = nullptr);

  /// @brief Desctructor
  /// Frees all allocated memory. Parent is not destroyed.
  ~ModifiedOptionsDialog();

  /// @brief Shows the dialog with provided details.
  /// @param details Details to use
  void show(ClientCore::CommitDetails & details);

private:

  /// @brief Table view
  QTableView * m_view;

  /// @brief Button box
  QDialogButtonBox * m_buttonBox;

  /// @brief Main layout
  QVBoxLayout * m_mainLayout;

  /// @brief "OK" button
  QPushButton * m_btOk;

}; // class ModifiedOptionsDialog

////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_ModifiedOptionsDialog_hpp
