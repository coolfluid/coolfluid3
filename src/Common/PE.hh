#ifndef COOLFluiD_Parallel_PE_hh
#define COOLFluiD_Parallel_PE_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/PEInterface.hh"
#include "Common/WorkerStatus.hh"

#ifdef CF_HAVE_MPI
#  include "Common/MPI/PEInterfaceMPI.hh"
   MyInt.cxx
   MyInt.hh
#endif // CF_HAVE_MPI

//////////////////////////////////////////////////////////////////////////////

namespace CF {

    namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// This class controls the Parallel environment
class Common_API PE {
public:

    /// Initialise the PE
    static void InitPE (int * argc, char *** args);
    /// Checks if the PE is initialized
    static bool IsInitialised ();
    /// Return a reference to the current PE
    static PEInterface<>& GetPE ();
    /// Free the PE
    static void DonePE ();
    
    /// Spawns processes on different host machines.
    /// @param count Number of processes to spawn.
    /// @param hosts Host list. Should be formatted as for -host option for mpirun
    /// (comma-separated items, without spaces - i.e. "host1,host2,host3"). May
    /// be null or empty (local host is used in these cases).
    /// @return Returns a MPI intercommunicator to allow the current process
    /// to communicate with created processes.
    /// @todo return a null comm or throw an exception on error (spawn failure,
    /// MPI env not initialized, count not valid...)
    static MPI_Comm spawn(unsigned int count, const char * hosts = CFNULL);
    
    /// Sets current process status.
    /// @param status New status
    static void setCurrentStatus(WorkerStatus::Type status);
    
    /// Gives the current process status.
    /// @return Returns the current process status
    static WorkerStatus::Type getCurrentStatus();
    
private:

    /// the current PE
    static PEInterface<> * m_curr_PE;

    /// Flag to keep track of Parallel Enviroment Initialized
    /// Note: cannot rely on m_curr_PE pointer because objects held by the enviroment
    ///       could not then check for initialization. An egg and chicken problem,
    ///       that appeared when using Log on the destructors of PEInterface related objects.
    static bool m_is_init;
    
    /// Path to the executable used to run the workers
    static char * m_command_workers;
    
    /// Current status. 
    /// Default value is @c #NOT_RUNNING.
    static WorkerStatus::Type m_current_status;

}; // end class PE

//////////////////////////////////////////////////////////////////////////////

  } // Common

} // COOLFluiD

#endif // COOLFluiD_Parallel_PE_hh
