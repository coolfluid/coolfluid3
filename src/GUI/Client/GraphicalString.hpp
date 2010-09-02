#ifndef CF_GUI_Client_ClientString_hpp
#define CF_GUI_Client_ClientString_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/GraphicalValue.hpp"

class QLineEdit;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class GraphicalString : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalString(QWidget * parent = 0);

    ~GraphicalString();

    virtual bool setValue(const QVariant & value);

    virtual QVariant getValue() const;

  private slots:

    void textUpdated(const QString & text);

  private:

    QLineEdit * m_lineEdit;

  }; // class GraphicalDouble

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientString_hpp
