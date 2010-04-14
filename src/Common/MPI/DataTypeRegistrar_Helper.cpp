#include "Common/MPI/DataTypeRegistrar_Helper.hpp"
#include "Common/MPI/DataTypeHandler.hpp"

namespace CF {
namespace Common  {
namespace MPI  {

////////////////////////////////////////////////////////////////////////////////

DataTypeRegistrar_Helper::DataTypeRegistrar_Helper ()
    : TheType (MPI_DATATYPE_NULL)
{
}

DataTypeRegistrar_Helper::~DataTypeRegistrar_Helper ()
{
}

void DataTypeRegistrar_Helper::DoRegister ()
{
    DataTypeHandler::GetHandler ().RegisterType (this);
}
////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF
