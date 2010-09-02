#ifndef CF_GUI_Client_ClientInt_hpp
#define CF_GUI_Client_ClientInt_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/GraphicalValue.hpp"

class QSpinBox;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class GraphicalInt : public GraphicalValue
  {
    Q_OBJECT

  public:

    GraphicalInt(bool isUint, QWidget * parent = 0);

    ~GraphicalInt();

    virtual bool setValue(const QVariant & value);

    virtual QVariant getValue() const;

  private slots:

    void integerChanged(int value);

  private:

    QSpinBox * m_spinBox;

    bool m_isUint;

  }; // class GraphicalInt

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientInt_hpp
