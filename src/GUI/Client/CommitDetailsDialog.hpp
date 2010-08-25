#ifndef CF_GUI_Client_CommitDetailsDialog_hpp
#define CF_GUI_Client_CommitDetailsDialog_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "Common/CF.hpp"

class QDialogButtonBox;
class QPushButton;
class QTableView;
class QVBoxLayout;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace GUI {
    namespace Client {

      class CommitDetails;

      /////////////////////////////////////////////////////////////////////////////

      /// @brief Dialog that shows commit details.
      class CommitDetailsDialog : public QDialog
      {
        Q_OBJECT

      public:

        /// @brief Constructor
        /// @param parent Parent widget. May be @c CFNULL.
        CommitDetailsDialog(QWidget * parent = CFNULL);

        /// @brief Desctructor
        /// Frees all allocated memory. Parent is not destroyed.
        ~CommitDetailsDialog();

        /// @brief Shows the dialog with provided details.
        /// @param details Details to use
        void show(CommitDetails & details);

      private:

        /// @brief Table view
        QTableView * m_view;

        /// @brief Button box
        QDialogButtonBox * m_buttonBox;

        /// @brief Main layout
        QVBoxLayout * m_mainLayout;

        /// @brief "OK" button
        QPushButton * m_btOk;

      }; // class CommitDetailsDialog

      ////////////////////////////////////////////////////////////////////////////////

    } // namespace Client
  } // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CommitDetailsDialog_hpp
