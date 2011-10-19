// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_ConfirmCommitDialog_h
#define cf3_GUI_Graphics_ConfirmCommitDialog_h

////////////////////////////////////////////////////////////////////////////////

#include <QDialog>
#include <QHash>
#include <QDialogButtonBox>

#include "UI/Graphics/LibGraphics.hpp"

class QPushButton;
class QLabel;
class QVBoxLayout;
class QTableView;
class QString;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {

namespace Core { class CommitDetails; }

namespace Graphics {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Dialog used to ask user whether modifcations have to be
  /// committed before continuing.

  /// This dialog is used, for instance, when user clicks on another node
  /// or closes the application without committing.
  class Graphics_API ConfirmCommitDialog : public QDialog
  {
    Q_OBJECT

  public:

    /// @brief Commit confirmation enum
    /// Indicates on which button the user clicked.
    enum CommitConfirmation
    {
      /// @brief The user clicked on "Commit".
      COMMIT,

      /// @brief The user clicked on "Don't commit"
      DONT_COMMIT,

      /// @brief The user clicked on "Yes"
      YES,

      /// @brief The user clicked on "No"
      NO,

      /// @brief The user clicked on "Cancel"
      CANCEL
    };

  public:

    /// @brief Constructor
    /// @param parent Dialog parent. May be null.
    ConfirmCommitDialog(QWidget * parent = nullptr);

    /// @brief Destructor.

    /// Free all allocated memory. Parent is not destroyed.
    ~ConfirmCommitDialog();

    /// @brief Shows the dialog
    /// @param commitDetails Commit details to show.
    /// @return Returns the user answer.
    ConfirmCommitDialog::CommitConfirmation show(Core::CommitDetails & commitDetails);

  private slots:

    /// @brief Slot called when a button is clicked.
    void buttonClicked();

  private: // methods

    /// @brief Creates a button
    /// @param text Button text.
    /// @param commConf Confimation type.
    /// @param role Button role.
    void createButton(const QString & text, CommitConfirmation commConf,
                      QDialogButtonBox::ButtonRole role);

  private: // data

    /// @brief Button box
    QDialogButtonBox * m_buttonBox;

    /// @brief Main layout.
    QVBoxLayout * m_mainLayout;

    /// @brief View to show the commit details.
    QTableView * m_detailsView;

    /// @brief Label for the text
    QLabel * m_labText;

    /// @brief User answer
    CommitConfirmation m_answer;

    /// @brief Dialog buttons
    QHash<CommitConfirmation, QPushButton *> m_buttons;

  }; // class ConfirmCommitDialog

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

//////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Graphics_ConfirmCommitDialog_h
