#ifndef MPIDATATYPEREGISTRAR_HELPER_HH
#define MPIDATATYPEREGISTRAR_HELPER_HH

#include "Common/MPI/MPIDataType.hh"
#include <mpi.h>

namespace CF {
    namespace Common  {
//////////////////////////////////////////////////////////////////////////////

class MPIDataTypeRegistrar_Helper : public MPIDataType
{
protected:
    MPI_Datatype TheType;

public:
    MPIDataTypeRegistrar_Helper ();
    virtual ~MPIDataTypeRegistrar_Helper ();


    void DoRegister ();

    MPI_Datatype GetType () const
    { return TheType; }

};

//////////////////////////////////////////////////////////////////////////////

   }
}

#endif
