#include "Common/MPI/DataTypeHandler.hpp"
#include "Common/Log.hpp"
#include <mpi.h>

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace MPI  {

////////////////////////////////////////////////////////////////////////////////

DataTypeHandler * DataTypeHandler::TheInstance = 0;

void DataTypeHandler::InitType (DataType * Type)
{
    Type->regist (Communicator);
}

////////////////////////////////////////////////////////////////////////////////

void DataTypeHandler::DoneType (DataType * Type)
{
    Type->unregist ();
}

////////////////////////////////////////////////////////////////////////////////

void DataTypeHandler::RegisterType (DataType * Type)
{
    Types.insert(Type);
    if (IsInitialized ())
        InitType (Type);
}

void DataTypeHandler::InitTypes ()
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

////////////////////////////////////////////////////////////////////////////////

void DataTypeHandler::DoneTypes()
{
  CFLogDebug("DataTypeHandler::DoneTypes() begin" << "\n");

  cf_assert(Initialized);

  ContainerType::const_iterator Iter;
  for (Iter = Types.begin ();  Iter != Types.end (); Iter++) {
    DoneType(*Iter);
  }

  Types.clear();

  Initialized = false;

  CFLogDebug("DataTypeHandler::DoneTypes() end" << "\n");
}

////////////////////////////////////////////////////////////////////////////////

bool DataTypeHandler::IsInitialized () const
{
  return Initialized;
}

////////////////////////////////////////////////////////////////////////////////

DataTypeHandler::DataTypeHandler (MPI_Comm Comm) :
    Communicator(Comm), Initialized(false)
{
  cf_assert (TheInstance == 0);
  TheInstance = this;
}

////////////////////////////////////////////////////////////////////////////////

DataTypeHandler::~DataTypeHandler()
{
  CFLogDebug( "DataTypeHandler::~DataTypeHandler (): " << Types.size () << " types to destroy" << "\n");

  if (IsInitialized ()) {
    DoneTypes ();
  }

  TheInstance = CFNULL;
}

////////////////////////////////////////////////////////////////////////////////

DataTypeHandler & DataTypeHandler::GetHandler()
{
  cf_assert (TheInstance != CFNULL);
  return *TheInstance;
}

////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF
