// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_ConfirmCommitDialog_h
#define cf3_ui_Graphics_ConfirmCommitDialog_h

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
namespace ui {

namespace core { class CommitDetails; }

namespace graphics {

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
    ConfirmCommitDialog::CommitConfirmation show(core::CommitDetails & commitDetails);

  private slots:

    /// @brief Slot called when a button is clicked.
    void button_clicked();

  private: // methods

    /// @brief Creates a button
    /// @param text Button text.
    /// @param commConf Confimation type.
    /// @param role Button role.
    void create_button(const QString & text, CommitConfirmation commConf,
                      QDialogButtonBox::ButtonRole role);

  private: // data

    /// @brief Button box
    QDialogButtonBox * m_button_box;

    /// @brief Main layout.
    QVBoxLayout * m_main_layout;

    /// @brief View to show the commit details.
    QTableView * m_details_view;

    /// @brief Label for the text
    QLabel * m_lab_text;

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

#endif // cf3_ui_Graphics_ConfirmCommitDialog_h
