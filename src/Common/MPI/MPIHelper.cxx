#include "Common/MPI/MPIHelper.hh"
#include "Common/MPI/MPIException.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
    namespace Common {

//////////////////////////////////////////////////////////////////////////////

void DoMPIError (int status)
{
    char Buf[MPI_MAX_ERROR_STRING+1];
    int BufSize = sizeof (Buf)-1;
    MPI_Error_string (status, Buf, &BufSize);
    std::string S ("MPI Error: ");
    S+=Buf;
    S+='\n';
    throw MPIException (FromHere(),S);
}

//////////////////////////////////////////////////////////////////////////////

    }
}
