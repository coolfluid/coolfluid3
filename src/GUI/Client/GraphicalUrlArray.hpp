#ifndef CF_GUI_Client_GraphicalArray
#define CF_GUI_Client_GraphicalArray

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/GraphicalValue.hpp"

class QListView;
class QPushButton;
class QStringListModel;
class QVBoxLayout;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  /////////////////////////////////////////////////////////////////////////

  class RemoteFSBrowser;

  class GraphicalUrlArray : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUrlArray(QWidget * parent = 0);

    ~GraphicalUrlArray();

    virtual QVariant getValue() const;

    virtual bool setValue(const QVariant & value);

  private slots:

    void btAddClicked();

    void btRemoveClicked();

  private:

    QStringListModel * m_model;

    QListView * m_listView;

    QPushButton * m_btAdd;

    QPushButton * m_btRemove;

    QVBoxLayout * m_buttonsLayout;

    RemoteFSBrowser * m_browser;

  };

  /////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

///////////////////////////////////////////////////////////////////////////


#endif // CF_GUI_Client_GraphicalArray
