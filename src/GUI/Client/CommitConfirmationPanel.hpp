#ifndef CF_GUI_Client_CommitConfirmationPanel_h
#define CF_GUI_Client_CommitConfirmationPanel_h

/////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"

#include "GUI/Client/CloseConfirmationPanel.hpp"

class QComboBox;

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

    struct CloseConfirmationInfos;

/////////////////////////////////////////////////////////////////////////////

    class CommitConfirmationPanel : public CloseConfirmationPanel
    {
      Q_OBJECT

    public:

      CommitConfirmationPanel(QDialog * parent = CFNULL);

      ~CommitConfirmationPanel();

      virtual bool isAccepted() const;

      virtual void getData(CloseConfirmationInfos & infos) const;

      virtual void setData(const CloseConfirmationInfos & infos);

    protected:

      virtual void hideComponents(bool hide);

      private slots:

      void showDetails();

    private:

      QComboBox * comboBox;

      QString details;

    }; // class CommitConfirmationPanel

////////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CommitConfirmationPanel_h
