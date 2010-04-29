#ifndef CF_Common_Provider_hpp
#define CF_Common_Provider_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/NamedObject.hpp"
#include "Common/ProviderBase.hpp"
#include "Common/Factory.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

  template < class BASE > class Factory;

////////////////////////////////////////////////////////////////////////////////

/// @brief Templated class provider
/// Used in the template abstract factory with Self Registering Objects.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE>
class Provider : public Common::NamedObject,
                 public Common::ProviderBase
{
public: // methods

  /// Constructor
  /// @param name provider registration name
  Provider(const std::string& name) :
    Common::NamedObject(name),
    Common::ProviderBase()
  {
    Common::Factory<BASE>::getInstance().regist(this);
  }

  /// Virtual destructor
  virtual ~Provider() {}

  /// @return the name of this provider
  virtual std::string getProviderName () const { return getName(); }

  /// @return the BASE of this provider
  virtual std::string getProviderType () const { return BASE::getClassName(); }

}; // end of class Provider

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////
#endif // CF_Common_Provider_hpp
