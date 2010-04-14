#ifndef CF_network_Network_h
#define CF_network_Network_h

////////////////////////////////////////////////////////////////////////////////

#include <QString>

#include "Common/Exception.hpp"
#include "Common/CodeLocation.hpp"

#include "GUI/Network/NetworkAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Exception thrown when the server can not open its socket.

  /// @author Quentin Gasper.

  class Network_API Network : public CF::Common::Exception
  {
    public:

    /// Constructor
    Network(const CF::Common::CodeLocation& where,
                     const std::string& what);

    /// Copy constructor
    Network(const NetworkException& e) throw ();

  }; // class Network

  /////////////////////////////////////////////////////////////////////////////

} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_network_Network_h
