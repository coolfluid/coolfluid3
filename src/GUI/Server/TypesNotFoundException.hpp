#ifndef CF_server_TypesNotFoundException_h
#define CF_server_TypesNotFoundException_h

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

/////////////////////////////////////////////////////////////////////////////

  /// @brief Exception thrown when types could not be loaded.
  
  /// @author Quentin Gasper.
  
  class TypesNotFoundException : public CF::Common::Exception
  {
  public:
    
    /// Constructor
    TypesNotFoundException(const CF::Common::CodeLocation& where,
                  const std::string& what);
    
    /// Copy constructor
    TypesNotFoundException(const TypesNotFoundException& e) throw ();
    
  }; // class TypesNotFound

////////////////////////////////////////////////////////////////////////////
  
} // namespace Server
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_server_TypesNotFoundException_h
