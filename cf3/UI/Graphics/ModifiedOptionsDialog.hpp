// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_ModifiedOptionsDialog_hpp
#define cf3_GUI_Graphics_ModifiedOptionsDialog_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "common/CF.hpp"

#include "UI/Graphics/LibGraphics.hpp"

class QDialogButtonBox;
class QPushButton;
class QTableView;
class QVBoxLayout;

/////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {

namespace Core { class CommitDetails; }

namespace Graphics {

////////////////////////////////////////////////////////////////////////////

/// @brief Dialog that shows commit details.
class Graphics_API ModifiedOptionsDialog : public QDialog
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
  void show(Core::CommitDetails & details);

private:

  /// @brief Table view
  QTableView * m_view;

  /// @brief Button box
  QDialogButtonBox * m_button_box;

  /// @brief Main layout
  QVBoxLayout * m_main_layout;

  /// @brief "OK" button
  QPushButton * m_bt_ok;

}; // class ModifiedOptionsDialog

////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_ModifiedOptionsDialog_hpp
