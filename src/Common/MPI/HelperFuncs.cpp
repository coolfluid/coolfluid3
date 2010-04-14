#include "Common/BasicExceptions.hpp"
#include "Common/MPI/HelperFuncs.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace MPI  {

////////////////////////////////////////////////////////////////////////////////

  void ThrowErrorHere (int status, const CodeLocation& here)
  {
      char Buf[MPI_MAX_ERROR_STRING+1];
      int BufSize = sizeof (Buf)-1;
      MPI_Error_string (status, Buf, &BufSize);
      std::string S ("MPI Error: ");
      S+=Buf;
      S+='\n';
      throw ParallelError ( here, S );
  }

////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF
