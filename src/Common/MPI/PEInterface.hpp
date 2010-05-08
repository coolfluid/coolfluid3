#ifndef CF_Common_PEInterface_hpp
#define CF_Common_PEInterface_hpp

#include "boost/mpi.hpp"
#include "Common/WorkerStatus.hpp"
#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Parallel Environment Interface
/// The whole interface depends on boost.mpi, it is basically an encapsulation of mpi::environment mpi::world and WorkerStatus
/// @author Tamas Banyai to blame

using namespace boost;

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Base class for the PEInterface
class Common_API PEInterface : public boost::noncopyable, public mpi::communicator {
public:

  /// public constructor
  PEInterface(int argc, char** args);

  /// destructor
  ~PEInterface();

  /// Return a reference to the current PE
  static PEInterface& getInstance();

  /// Initialise the PE
  void init(int argc, char** args);

  /// Checks if the PE is initialized
  bool is_init();

  /// Free the PE, careful because some mpi-s fail upon re-init after a proper finalize
  void finalize();

  /// Return rank, additionally, if is_init==0 returns 0
  Uint rank();

  /// Sets current process status.
  /// @param status New status
  /// @todo the name WorkerStatus is inappropriate, better to name it for example ProcessStatus
  void change_status(WorkerStatus::Type status);

  /// Gives the current process status.
  /// @return Returns the current process status
  WorkerStatus::Type status();

  /// Operator to boost.mpi environment, environment is noncopyable
  operator mpi::environment&() { return *m_environment; };
  operator mpi::environment*() { return m_environment; };

private:

  /// private constructor
  PEInterface();

  /// Current status.
  /// Default value is @c #NOT_RUNNING.
  WorkerStatus::Type m_current_status;

  /// mpi environment
  mpi::environment *m_environment;

}; // PEInterface

////////////////////////////////////////////////////////////////////////////////

  } // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PEInterface_hpp
