#ifndef CF_Common_PEInterface_hpp
#define CF_Common_PEInterface_hpp

#include "Common/CF.hpp"
#include "Common/NonCopyable.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Base class for the PEInterface
class Common_API PEInterface : public NonCopyable<PEInterface> {
public:

  /// virtual destructor
  virtual ~PEInterface ();

  /// Returns the total number of execution contexts in the universum
  virtual Uint get_procesor_count () const = 0;

  /// Return the ID of this processor (between 0 and get_procesor_count)
  virtual Uint get_rank () const = 0;

  /// Set a barrier
  virtual void set_barrier() = 0;

  /// Return true if this is a parallel simulation in some way
  /// Running only 1 processor is NOT parallel
  virtual bool is_parallel () const = 0;

  /// return the name of the model
  virtual std::string getModelName () const = 0;

}; // PEInterface

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PEInterface_hpp
