#ifndef CF_GUI_Client_GraphicalUrl_hpp
#define CF_GUI_Client_GraphicalUrl_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/GraphicalValue.hpp"

class QCompleter;
class QLineEdit;
class QPushButton;
class QString;
class QStringListModel;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////

  class GraphicalUrl : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalUrl(QWidget *parent = 0);

    ~GraphicalUrl();

    virtual bool setValue(const QVariant & path);

    virtual QVariant getValue() const;

  private slots:

    void btBrowseClicked();

    void updateModel(const QString & path);

  private:

    QLineEdit * m_editPath;

    QPushButton * m_btBrowse;

    QCompleter * m_completer;

    QStringListModel * m_completerModel;

  }; // class GraphicalUrl

////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_GraphicalUrl_hpp
