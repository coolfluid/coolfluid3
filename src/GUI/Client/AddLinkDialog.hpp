#ifndef CF_GUI_Client_AddLinkDialog_hpp
#define CF_GUI_Client_AddLinkDialog_hpp

//////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "Common/CF.hpp"

class QDialogButtonBox;
class QDomDocument;
class QLabel;
class QLineEdit;
class QModelIndex;
class QTreeView;
class QVBoxLayout;

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  ////////////////////////////////////////////////////////////////////////////

  /// @brief Allows the user to create a link component.

  class AddLinkDialog : public QDialog
  {
    Q_OBJECT

    public :

      /// @brief Constructor.

      /// The tree model is set to @c CFNULL.
      /// @param parent Parent widget. May be null.

      AddLinkDialog(QWidget * parent = CFNULL);

      /// @brief Destructor.

      /// Frees all allocated memory. The tree model is not destroyed.
      ~AddLinkDialog();

      /// @brief Displays the dialog.

      /// This is a blocking method. It will not return until the dialog is
      /// visible. Futhermore, this method guarantees that it will return
      /// @c true only if user clicked on "OK", entered a name and selected a
      /// target. Otherwise it will return @c false and @c target and @c name
      /// are not modified.
      /// @param root Index of simulation tree root. This index must be a
      /// part of the model.
      /// @param target Reference to an object where the selected index will be
      /// written to.
      /// @param name Reference to a string where the name of the new link will
      /// be written to.
      /// @return Returns @c true if user clicked "OK" after providing wanted
      /// information.
      bool show(const QModelIndex & root, QModelIndex & target,
                QString & name);

      /// @brief Sets the tree model.

      /// The old model, if any, is not destroyed.
      /// @param treeModel The new model. May be null.
      void setTreeModel(const QDomDocument & tree/*TreeModel * treeModel*/);

    private slots :

      void btOkClicked();

      void btCancelClicked();

    private :

      /// Label for the name
      QLabel * m_labName;

      /// Label for the target
      QLabel * m_labTarget;

      /// Text edit for the name
      QLineEdit * m_txtName;

      /// Box for the buttons
      QDialogButtonBox * m_buttons;

      /// Tree view
      QTreeView * m_treeView;

      QVBoxLayout * m_mainLayout;

      bool m_okCLicked;

  }; // class AddLinkDialog

  ////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_AddLinkDialog_hpp
