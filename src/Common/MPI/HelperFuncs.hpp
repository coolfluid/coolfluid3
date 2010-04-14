#ifndef CF_Common_MPI_HelperFuncs_HH
#define CF_Common_MPI_HelperFuncs_HH

#include <mpi.h>

#include "Common/CommonAPI.hpp"
#include "Common/CodeLocation.hpp"

namespace CF {
namespace Common  {
namespace MPI  {

////////////////////////////////////////////////////////////////////////////////

/// Throw a ParallelError with location information
Common_API
void ThrowErrorHere ( int status, const CodeLocation& here );

/// Macro to do the checking and call the error if necessary
#define CheckMPIStatus(call) { int status = call ; if ( status != MPI_SUCCESS ) ThrowErrorHere(status,FromHere()); }

////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

#endif // CF_Common_MPI_HelperFuncs_HH
