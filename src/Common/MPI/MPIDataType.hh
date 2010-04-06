#ifndef MPIDATATYPE_HH
#define MPIDATATYPE_HH

#include <mpi.h>

namespace CF {
  namespace Common  {

//////////////////////////////////////////////////////////////////////////////

/// Interface for MPI Datatypes
class MPIDataType {
public:

  /// Regist the Datatype
  virtual void Register(MPI_Comm Comm) = 0;

  /// Unregist the Datatype
  virtual void UnRegister() = 0;

  /// Virtual destructor
  virtual ~MPIDataType ();

}; // end class MPIDataType

//////////////////////////////////////////////////////////////////////////////

  } // Common
} // COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // MPIDATATYPE_HH
