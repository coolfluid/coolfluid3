#ifndef PARALLEL_MPI_MPIEXCEPTION_HH
#define PARALLEL_MPI_MPIEXCEPTION_HH

#include "Common/Exception.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
   namespace Common  {

//////////////////////////////////////////////////////////////////////////////

class MPIException : public Common::Exception {
public:

  /// Constructor
  MPIException (const Common::CodeLocation& where, const std::string& what) :
    Common::Exception(where, what, "MPIException") {}

  /// Copy constructor
  MPIException(const MPIException& e) throw  () : Exception(e) {}

}; // class MPIException

//////////////////////////////////////////////////////////////////////////////

    } // namespace Common
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // PARALLEL_MPI_MPIEXCEPTION_HH
