#ifndef CF_server_UnknownClientId_h
#define CF_server_UnknownClientId_h

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {
      
/////////////////////////////////////////////////////////////////////////////

  /// @brief Exception thrown when a given client id could not be associated to
  /// any existing client.
  
  /// @author Quentin Gasper.
  class UnknownClientIdException : public CF::Common::Exception
  {
  public:
    
    /// Constructor
    UnknownClientIdException(const CF::Common::CodeLocation& where,
                    const std::string& what);
    
    /// Copy constructor
    UnknownClientIdException(const UnknownClientIdException& e) throw ();
    
  }; // class UnknownClientId

////////////////////////////////////////////////////////////////////////////
  
} // namespace Server
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_server_UnknownClientId_h
