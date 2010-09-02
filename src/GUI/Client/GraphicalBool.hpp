#ifndef CF_GUI_Client_ClientBool_hpp
#define CF_GUI_Client_ClientBool_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/GraphicalValue.hpp"

class QCheckBox;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class GraphicalBool : public GraphicalValue
  {
    Q_OBJECT
  public:

    GraphicalBool(QWidget * parent = 0);

    ~GraphicalBool();

    virtual bool setValue(const QVariant & value);

    virtual QVariant getValue() const;

  private slots:

    void stateChanged(int state);

  private:

      QCheckBox * m_checkBox;

  }; // class GraphicalBool

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientBool_hpp
