#ifndef DataTypeREGISTRAR_HELPER_HH
#define DataTypeREGISTRAR_HELPER_HH

#include "Common/MPI/DataType.hh"

namespace CF {
namespace Common  {
namespace MPI {

//////////////////////////////////////////////////////////////////////////////

class DataTypeRegistrar_Helper : public MPI::DataType
{
protected:
    MPI_Datatype TheType;

public:
    DataTypeRegistrar_Helper ();
    virtual ~DataTypeRegistrar_Helper ();


    void DoRegister ();

    MPI_Datatype GetType () const
    { return TheType; }

};

//////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

#endif
