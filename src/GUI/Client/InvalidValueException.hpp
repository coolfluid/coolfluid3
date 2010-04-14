#ifndef CF_GUI_Client_InvalidValue_h
#define CF_GUI_Client_InvalidValue_h

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {


////////////////////////////////////////////////////////////////////////////////

  /// @brief Exception thrown by @c GraphicalOption class when a QVariant
  /// object can not be convert to the desired type of data.

  /// @author Quentin Gasper.

  class InvalidValue : public CF::Common::Exception
  {
    public:

    /// Constructor
    InvalidValue(const CF::Common::CodeLocation& where,
    const std::string& what);

    /// Copy constructor
    InvalidValue(const InvalidValueException& e) throw ();

  }; // class InvalidValue

////////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_InvalidValue_h
