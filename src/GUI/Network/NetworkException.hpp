#ifndef CF_network_NetworkException_h
#define CF_network_NetworkException_h

/////////////////////////////////////////////////////////////////////////////

#include <QString>

#include "Common/Exception.hh"
#include "Common/CodeLocation.hh"

#include "GUI/Network/NetworkAPI.hh"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {
   
/////////////////////////////////////////////////////////////////////////////

  /// @brief Exception thrown when the server can not open its socket.
  
  /// @author Quentin Gasper.
  
  class Network_API NetworkException : public CF::Common::Exception
  {
    public:
    
    /// Constructor
    NetworkException(const CF::Common::CodeLocation& where, 
                     const std::string& what);
    
    /// Copy constructor
    NetworkException(const NetworkException& e) throw ();
    
  }; // class NetworkException
  
  /////////////////////////////////////////////////////////////////////////////
  
} // namespace Network 
} // namespace GUI 
} // namespace CF
  
/////////////////////////////////////////////////////////////////////////////

#endif // CF_network_NetworkException_h
