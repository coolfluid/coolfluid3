#ifndef CF_GUI_Client_ClientDouble_hpp
#define CF_GUI_Client_ClientDouble_hpp

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/GraphicalValue.hpp"

class QDoubleValidator;
class QLineEdit;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class GraphicalDouble : public GraphicalValue
  {

  public:

      GraphicalDouble(QWidget * parent = 0);

      ~GraphicalDouble();

      virtual bool setValue(const QVariant & value);

      virtual QVariant getValue() const;

  private:

      QLineEdit * m_lineEdit;

      QDoubleValidator * m_validator;

  }; // class GraphicalDouble

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientDouble_hpp
