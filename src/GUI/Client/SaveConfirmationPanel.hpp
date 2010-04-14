#ifndef CF_GUI_Client_SaveConfirmationPanel_h
#define CF_GUI_Client_SaveConfirmationPanel_h

////////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/CloseConfirmationPanel.hpp"

class QCheckBox;
class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QWidget;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  /////////////////////////////////////////////////////////////////////////////

  struct CloseConfirmationInfos;

  /////////////////////////////////////////////////////////////////////////////

  class SaveConfirmationPanel : public CloseConfirmationPanel
  {
    Q_OBJECT

    public:

    SaveConfirmationPanel(QDialog * parent = NULL, bool becauseCommit = false);

    ~SaveConfirmationPanel();

    virtual bool isAccepted() const;

    virtual void getData(CloseConfirmationInfos & infos) const;

    virtual void setData(const CloseConfirmationInfos & infos);

    protected:

    virtual void hideComponents(bool hide);

    private slots:

    void currentIndexChanged(int index);

    private:

    QComboBox * m_comboBox;

    QCheckBox * m_checkSaveLocallyOnError;

    QLabel * m_labStatus;

    QWidget * m_fileNameWidget;

    QLineEdit * m_editFileName;

    QHBoxLayout * m_widgetLayout;

    QString m_filename;

    bool m_rollingBack;

    int m_previousIndex;

    QString saveLocally();

    QString saveRemotely();

  };

  /////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_SaveConfirmationPanel_h
