#ifndef CF_Common_PEInterface_hh
#define CF_Common_PEInterface_hh

#include <string>

#include "Common/CommonAPI.hh"

#include "Common/BasicExceptions.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

    namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// Base class for the PEInterface
class Common_API PEInterfaceBase {
public:

  /// destructor
  ~PEInterfaceBase ();

  /// Returns the total number of execution contexts in
  /// the universum (not the big one of course, but the
  /// cluster-one ;-) )
  unsigned int GetProcessorCount () const;

  /// Return the ID of this processor (between 0 and GetProcessorCount)
  unsigned int GetRank () const;

  /// Set the barrier
  void setBarrier();

  /// Return true if this is a parallel simulation in some way
  /// (IMPORTANT: When CF was compiled for multiple CPU's
  ///  - either MPI or SHM - but is started/called using only ONE
  ///  cpu, the run is NOT considered parallel!)
  bool IsParallel () const;

  /// returns the name of this model
  std::string GetName () const;

  /// return true if the model is multi-cpu capable
  bool IsParallelCapable () const;

  /// This function should be called periodically to help advance
  /// pending communication requests.
  void AdvanceCommunication ();

}; // end class PEInterfaceBase

//////////////////////////////////////////////////////////////////////////////

  } // Common

} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PEInterface_hh
