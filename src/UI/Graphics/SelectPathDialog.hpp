// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_SelectPathDialog_hpp
#define CF_GUI_Graphics_SelectPathDialog_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "UI/Graphics/LibGraphics.hpp"

class QCompleter;
class QDialogButtonBox;
class QLineEdit;
class QModelIndex;
class QStringListModel;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common { class URI; }

namespace UI {
namespace Graphics {

  //////////////////////////////////////////////////////////////////////////

  class TreeView;

  class Graphics_API SelectPathDialog : public QDialog
  {
      Q_OBJECT
  public:

      SelectPathDialog(QWidget *parent = 0);

      CF::Common::URI show(const CF::Common::URI & path);

  private slots:

      /// @brief Slot called when "OK" button is clicked.

      /// Sets @c #m_okClicked to @c true and then sets
      /// the dialog to an invisible state.
      void btOkClicked();

      /// @brief Slot called when "Cancel" button is clicked.

      /// Sets @c #m_okClicked to @c false and then sets the dialog to an
      /// invisible state.
      void btCancelClicked();

      void itemClicked(const QModelIndex & index);

      void pathChanged(const QString & index);

  private:

      QVBoxLayout * m_mainLayout;

      TreeView * m_treeView;

      QLineEdit * m_editPath;

      QDialogButtonBox * m_buttons;

      bool m_okClicked;

      QCompleter * m_completer;

      QStringListModel * m_model;

      bool m_nodeClicked;

  }; // class SelectPathDialog

  //////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Graphics_SelectPathDialog_hpp
