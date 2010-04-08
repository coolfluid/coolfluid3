#ifndef CF_Common_MPI_DATATYPE_HH
#define CF_Common_MPI_DATATYPE_HH

#include <mpi.h>

#include "Common/CommonAPI.hh"

namespace CF {
namespace Common  {
namespace MPI  {

//////////////////////////////////////////////////////////////////////////////

/// Interface for MPI Datatypes
class Common_API DataType {
public:

  /// Virtual destructor
  virtual ~DataType ();

  /// Regist the Datatype
  virtual void regist( MPI_Comm Comm ) = 0;

  /// Unregist the Datatype
  virtual void unregist () = 0;

}; // DataType

//////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_DATATYPE_HH
