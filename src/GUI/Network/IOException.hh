#ifndef CF_network_IOException_h
#define CF_network_IOException_h

/////////////////////////////////////////////////////////////////////////////

#include <QString>

#include "Common/Exception.hh"

#include "GUI/Network/NetworkAPI.hh"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {
   
/////////////////////////////////////////////////////////////////////////////
    
  /// @brief Exception thrown when an In/Out error occurs.
  
  /// @author Quentin Gasper.
  
  class Network_API IOException : public CF::Common::Exception
  {
    
    public:
    
    /// Constructor
    IOException(const CF::Common::CodeLocation& where, const std::string& what);
    
    /// Copy constructor
    IOException(const IOException& e) throw ();
    
  }; // class IOException
    
/////////////////////////////////////////////////////////////////////////////
    
} // namespace Network 
} // namespace GUI 
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_network_IOException_h
