#ifndef CF_GUI_Client_SelectPathPanel_hpp
#define CF_GUI_Client_SelectPathPanel_hpp

////////////////////////////////////////////////////////////////////////////

#include <QWidget>

class QHBoxLayout;
class QLineEdit;
class QPushButton;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////

  class SelectPathPanel : public QWidget
  {
    Q_OBJECT

  public:

    SelectPathPanel(const QString & path = QString(), QWidget *parent = 0);

    ~SelectPathPanel();

    QString getValueString() const;

    void setValue(const QString & path);

  private slots:

    void btBrowseClicked();

  private:

    QLineEdit * m_editPath;

    QPushButton * m_btBrowse;

    QHBoxLayout * m_layout;

  }; // class SelectPathPanel

////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_SelectPathPanel_hpp
