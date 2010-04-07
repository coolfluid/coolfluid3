#ifndef COOFluiD_Common_ProviderBase_hh
#define COOFluiD_Common_ProviderBase_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hh"
#include "Common/NonCopyable.hh"

#include "Common/CommonAPI.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// @brief Base class for provider types
/// @author Dries Kimpe
/// @author Tiago Quintino
class Common_API ProviderBase : public Common::NonCopyable<ProviderBase> {

public: // methods

  /// Constructor
  ProviderBase ();

  /// Virtual destructor
  virtual ~ProviderBase ();

  /// Free an instance created by this factory
  /// @param ptr pointer to be freed
  virtual void freeInstance ( void * ptr ) = 0;

  /// @return the name of this provider
  virtual std::string getProviderName () const = 0;

  /// @return the type of this provider
  virtual std::string getProviderType () const = 0;

}; // end ProviderBase

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // COOFluiD_Common_ProviderBase_hh
