// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_SelectPathDialog_hpp
#define cf3_ui_Graphics_SelectPathDialog_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "ui/graphics/LibGraphics.hpp"

class QCompleter;
class QDialogButtonBox;
class QLineEdit;
class QModelIndex;
class QStringListModel;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace common { class URI; }

namespace ui {
namespace graphics {

  //////////////////////////////////////////////////////////////////////////

  class TreeView;

  class Graphics_API SelectPathDialog : public QDialog
  {
      Q_OBJECT
  public:

      SelectPathDialog(QWidget *parent = 0);

      cf3::common::URI show(const cf3::common::URI & path);

  private slots:

      /// @brief Slot called when "OK" button is clicked.

      /// Sets @c #m_okClicked to @c true and then sets
      /// the dialog to an invisible state.
      void bt_ok_clicked();

      /// @brief Slot called when "Cancel" button is clicked.

      /// Sets @c #m_okClicked to @c false and then sets the dialog to an
      /// invisible state.
      void bt_cancel_clicked();

      void item_clicked(const QModelIndex & index);

      void path_changed(const QString & index);

  private:

      QVBoxLayout * m_main_layout;

      TreeView * m_tree_view;

      QLineEdit * m_edit_path;

      QDialogButtonBox * m_buttons;

      bool m_ok_clicked;

      QCompleter * m_completer;

      QStringListModel * m_model;

      bool m_node_clicked;

  }; // class SelectPathDialog

  //////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_SelectPathDialog_hpp
