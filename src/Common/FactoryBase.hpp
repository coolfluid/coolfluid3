#ifndef CF_Common_FactoryBase_hpp
#define CF_Common_FactoryBase_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/NonCopyable.hpp"
#include "Common/CF.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common { class ProviderBase; }

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class serves as a base class for factory's so that all factories
/// can be held by a FactoryRegistry
/// @author Tiago Quintino
class Common_API FactoryBase :
  public Common::NonCopyable<FactoryBase> {

public: // methods

  /// @return the name of the type of this factory
  virtual std::string getTypeName() const = 0;

  /// @return all the providers in this Factory in a std::vector
  virtual std::vector<Common::ProviderBase*> getAllProviders() = 0;

}; // end class FactoryBase

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_FactoryBase_hpp
