#include "Common/MPI/MPIDataTypeRegistrar_Helper.hh"
#include "Common/MPI/MPIDataTypeHandler.hh"

namespace CF {
   namespace Common  {

//////////////////////////////////////////////////////////////////////////////

MPIDataTypeRegistrar_Helper::MPIDataTypeRegistrar_Helper ()
	    : TheType (MPI_DATATYPE_NULL)
{
}

MPIDataTypeRegistrar_Helper::~MPIDataTypeRegistrar_Helper ()
{
}

void MPIDataTypeRegistrar_Helper::DoRegister ()
{
    MPIDataTypeHandler::GetHandler ().RegisterType (this);
}
//////////////////////////////////////////////////////////////////////////////
    }
}
