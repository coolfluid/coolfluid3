#ifndef CF_GUI_Client_SelectPathDialog_hpp
#define CF_GUI_Client_SelectPathDialog_hpp

////////////////////////////////////////////////////////////////////////////

#include <QDialog>

class QCompleter;
class QDialogButtonBox;
class QLineEdit;
class QModelIndex;
class QStringListModel;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common { class CPath; }

namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class TreeView;

  class SelectPathDialog : public QDialog
  {
      Q_OBJECT
  public:

      SelectPathDialog(QWidget *parent = 0);

      CF::Common::CPath show(const CF::Common::CPath & path);

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

  }; // class SelectPathDialog

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_SelectPathDialog_hpp
