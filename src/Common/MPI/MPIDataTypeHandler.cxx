#include "Common/MPI/MPIDataTypeHandler.hh"
#include "Common/Log.hh"
#include <mpi.h>

//////////////////////////////////////////////////////////////////////////////

namespace CF {

    namespace Common {

//////////////////////////////////////////////////////////////////////////////

MPIDataTypeHandler * MPIDataTypeHandler::TheInstance = 0;

void MPIDataTypeHandler::InitType (MPIDataType * Type)
{
    Type->Register (Communicator);
}

//////////////////////////////////////////////////////////////////////////////

void MPIDataTypeHandler::DoneType (MPIDataType * Type)
{
    Type->UnRegister ();
}

//////////////////////////////////////////////////////////////////////////////

void MPIDataTypeHandler::RegisterType (MPIDataType * Type)
{
    Types.insert(Type);
    if (IsInitialized ())
        InitType (Type);
}

void MPIDataTypeHandler::InitTypes ()
{
    cf_assert (!Initialized);
    ContainerType::const_iterator Iter = Types.begin ();
    while (Iter != Types.end ())
    {
        InitType (*Iter);
        Iter++;
    }
    Initialized = true;
}

//////////////////////////////////////////////////////////////////////////////

void MPIDataTypeHandler::DoneTypes()
{
  CFLogDebug("MPIDataTypeHandler::DoneTypes() begin" << "\n");

  cf_assert(Initialized);

  ContainerType::const_iterator Iter;
  for (Iter = Types.begin ();  Iter != Types.end (); Iter++) {
    DoneType(*Iter);
  }

  Types.clear();

  Initialized = false;

  CFLogDebug("MPIDataTypeHandler::DoneTypes() end" << "\n");
}

//////////////////////////////////////////////////////////////////////////////

bool MPIDataTypeHandler::IsInitialized () const
{
  return Initialized;
}

//////////////////////////////////////////////////////////////////////////////

MPIDataTypeHandler::MPIDataTypeHandler (MPI_Comm Comm) :
    Communicator(Comm), Initialized(false)
{
  cf_assert (TheInstance == 0);
  TheInstance = this;
}

//////////////////////////////////////////////////////////////////////////////

MPIDataTypeHandler::~MPIDataTypeHandler()
{
  CFLogDebug( "MPIDataTypeHandler::~MPIDataTypeHandler (): " << Types.size () << " types to destroy" << "\n");

  if (IsInitialized ()) {
    DoneTypes ();
  }

  TheInstance = CFNULL;
}

//////////////////////////////////////////////////////////////////////////////

MPIDataTypeHandler & MPIDataTypeHandler::GetHandler()
{
  cf_assert (TheInstance != CFNULL);
  return *TheInstance;
}

//////////////////////////////////////////////////////////////////////////////

  } // Common

} // COOLFluiD
