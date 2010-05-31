#ifndef CF_GUI_Client_CommitDetailsDialog_hpp
#define CF_GUI_Client_CommitDetailsDialog_hpp

/////////////////////////////////////////////////////////////////////////////

#include <QDialog>

#include "Common/CF.hpp"

class QPushButton;
class QDialogButtonBox;
class QVBoxLayout;
class QTableView;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

class CommitDetails;

/////////////////////////////////////////////////////////////////////////////

    class CommitDetailsDialog : public QDialog
    {
      Q_OBJECT

      public:

        CommitDetailsDialog(QWidget * parent = CFNULL);

        ~CommitDetailsDialog();

        void setCommitDetails(CommitDetails * details);

        void show(CommitDetails & details);

      public slots:

        void show();

      private:

        QPushButton * m_btOk;

        QTableView * m_view;

        QDialogButtonBox * m_buttonBox;

        QVBoxLayout * m_mainLayout;

        CommitDetails * m_commitDetails;

    }; // class CommitDetailsDialog

////////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CommitDetailsDialog_hpp
