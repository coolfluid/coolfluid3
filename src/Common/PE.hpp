#ifndef CF_Parallel_PE_hpp
#define CF_Parallel_PE_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/PEInterface.hpp"
#include "Common/WorkerStatus.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class controls the Parallel environment
/// @todo remove direct references to MPI from here
class Common_API PE : Common::NonCopyable<PE> {
public:

  /// Return a reference to the current PE
  static PE& getInstance ();

  /// Return a reference to the current PEInterface
  /// @pre init() must have been called
  /// @pre is_init() returns true
  static PEInterface& interface ();

  /// Initialise the PE
  void init (int * argc, char *** args);

  /// Checks if the PE is initialized
  bool is_init ();

  /// Gets the rank of the processor if already MPI is init
  /// else returns zero
  Uint get_rank () const;

  /// Free the PE
  void finalize ();

  /// Sets current process status.
  /// @param status New status
  void change_status (WorkerStatus::Type status);

  /// Gives the current process status.
  /// @return Returns the current process status
  WorkerStatus::Type status ();

private:

  /// private constructor
  PE();

  /// the current PE
  PEInterface * m_pe_interface;

  /// Flag to keep track of Parallel Enviroment initialized
  /// Note: cannot rely on m_curr_PE pointer because objects held by the enviroment
  ///       could not then check for initialization. An egg and chicken problem,
  ///       that appeared when using Log on the destructors of PEInterface related objects.
  bool m_is_init;

  /// Current status.
  /// Default value is @c #NOT_RUNNING.
  WorkerStatus::Type m_current_status;

}; // end class PE

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif // CF_Parallel_PE_hpp
