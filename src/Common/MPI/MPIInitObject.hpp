#ifndef PARALLEL_MPI_MPIINITOBJECT_hpp
#define PARALLEL_MPI_MPIINITOBJECT_hpp

#include <mpi.h>

namespace CF {
namespace Common  {
namespace MPI  {

////////////////////////////////////////////////////////////////////////////////

/// Interface to help other MPI dependent objects to initialize and cleanup
/// when MPI becomes available or is about to be unavailable.
class MPIInitObject
{
public:
    /// This function is called the moment MPI is fully initialized
    virtual void MPI_Init (MPI_Comm Communicator) = 0;

    /// This function is called *before* MPI deinitialization takes place.
    virtual void MPI_Done () = 0;

    virtual ~MPIInitObject ()
    {
    }
};

////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

#endif
