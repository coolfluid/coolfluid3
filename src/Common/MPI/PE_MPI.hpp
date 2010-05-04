#ifndef CF_Common_MPI_PE_MPI_hpp
#define CF_Common_MPI_PE_MPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include <mpi.h>

#include "Common/MPI/PEInterface.hpp"
#include "Common/MPI/DataTypeHandler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace MPI {

  class MPIInitObject;

////////////////////////////////////////////////////////////////////////////////

/// Parallel Environment interface for MPI
/// Cannot be in the MPI namespace because it is a specialization of
/// a class in the Common namespace
class Common_API PE_MPI : public PEInterface
{

private: // data

  /// typedef for initializator container
  typedef std::list<MPIInitObject *> InitContainerType;

  /// communicator
  MPI_Comm m_comm;

  // DataTypeHandler DataTypeHandler;

  std::list<MPIInitObject *> m_init_list;
  bool m_init_ok;
  bool m_stop_called;

public:

  /// constructor
  /// Needs argc, args because of MPI
  PE_MPI (int * argc, char *** args, MPI_Comm UsedCom = MPI_COMM_WORLD);

  /// Destructor
  ~PE_MPI ();

  /// Returns the total number of execution contexts in the universum
  virtual Uint get_procesor_count () const;

  /// Return the ID of this processor (between 0 and get_procesor_count)
  virtual Uint get_rank () const;

  /// Set a barrier
  virtual void set_barrier() {   MPI_Barrier(getCommunicator());  }

  /// Return true if this is a parallel simulation in some way
  /// Running only 1 processor is NOT parallel
  virtual bool is_parallel () const;

  /// return the name of the model
  virtual std::string getModelName () const;

  /// init- en finalize functions
  /// The object will be freed by the PE (should
  /// become a ModulePointer)
  void registInitObj (MPIInitObject * Obj);

  /// Return the communicator to be used
  /// (for now we always work in MPI_COMM_WORLD, but this could change
  MPI_Comm getCommunicator () const;

  /// Spawns processes on different host machines.
  /// @param count Number of processes to spawn.
  /// @param hosts Host list. Should be formatted as for -host option for mpirun
  /// (comma-separated items, without spaces - i.e. "host1,host2,host3"). May
  /// be null or empty (local host is used in these cases).
  /// @return Returns a MPI intercommunicator to allow the current process
  /// to communicate with created processes.
  /// @todo return a null comm or throw an exception on error (spawn failure,
  /// MPI env not initialized, count not valid...)
  MPI_Comm spawn( unsigned int count, const char * hosts = CFNULL);

protected:

  void CallInitFunctions ();
  void CallDoneFunctions ();
  void CallInitFunction (MPIInitObject * M) const;
  void CallDoneFunction (MPIInitObject * M) const;

private:

  MPI_Errhandler m_error_handler;

  /// Path to the executable used to run the workers
  char * m_command_workers;

}; // PE_MPI


////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_PE_MPI_hpp
