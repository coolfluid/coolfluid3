#ifndef CF_network_MalformedData_h
#define CF_network_MalformedData_h

////////////////////////////////////////////////////////////////////////////////

#include <QString>

#include "Common/Exception.hpp"

#include "GUI/Network/NetworkAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Exception thrown when data do not have excpected format.

  /// @author Quentin Gasper.

  class Network_API MalformedData : public CF::Common::Exception
  {

    public:

    /// Constructor
    MalformedData(const CF::Common::CodeLocation& where,
                           const std::string& what);

    /// Copy constructor
    MalformedData(const MalformedDataException& e) throw ();

  }; // class

  /////////////////////////////////////////////////////////////////////////////

} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_network_MalformedData_h
