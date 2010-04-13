#ifndef CF_GUI_Client_UnknownTypeException_h
#define CF_GUI_Client_UnknownTypeException_h

/////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hh"

/////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {
   
/////////////////////////////////////////////////////////////////////////////
    
  class UnknownTypeException : public CF::Common::Exception
  {
    public:
    
    /// Constructor
    UnknownTypeException(const CF::Common::CodeLocation& where, 
    const std::string& what);
    
    /// Copy constructor
    UnknownTypeException(const UnknownTypeException& e) throw ();
    
    
  }; // class UnknownTypeException
  
  /////////////////////////////////////////////////////////////////////////////
  
} // namespace Client
} // namespace GUI 
} // namespace CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UnknownTypeException_h
