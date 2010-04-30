#ifndef CF_Common_EventHandler_hpp
#define CF_Common_EventHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/DynamicObject.hpp"
#include "Common/OwnedObject.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Global Event Handler class
/// @author Tiago Quintino
class Common_API EventHandler :
    public Common::OwnedObject,
    public Common::DynamicObject,
    public boost::noncopyable
{
public: // methods

  /// Constructor private because is singleton
  EventHandler();

  /// Destructor private because is singleton
  ~EventHandler();

  /// Regists a signal on this EventHandler
  template < typename PTYPE, typename FTYPE >
  void addListener ( const std::string& sname, PTYPE* ptr, FTYPE pfunc, const std::string& desc = "" )
  {
    regist_signal ( sname , desc )->connect ( boost::bind ( pfunc, ptr, _1 ) );
  }

}; // class EventHandler

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_EventHandler_hpp
