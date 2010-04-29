#ifndef DataTypeHANDLER_hpp
#define DataTypeHANDLER_hpp

// This class handles correct initialisation and destruction
// of the registered datatypes

#include <mpi.h>
#include "Common/MPI/DataTypeRegistrar.hpp"
#include "Common/MPI/DataType.hpp"

#include <set>

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace MPI  {

////////////////////////////////////////////////////////////////////////////////

/// This class handles correct initialisation and destruction
/// of the registered datatypes
class DataTypeHandler {
public:

    static DataTypeHandler & GetHandler ();

    template <class CTYPE>
    static MPI_Datatype GetType ();

    DataTypeHandler (MPI_Comm Comm);
    ~DataTypeHandler ();

    void RegisterType (DataType * Type);

    bool IsInitialized () const;

    void InitTypes ();
    void DoneTypes ();

private:


    void InitType (DataType * T);
    void DoneType (DataType * T);


    MPI_Comm Communicator;
    bool Initialized;

    static DataTypeHandler * TheInstance;

    typedef std::set<DataType *> ContainerType;

    ContainerType Types;

}; // end class DataTypeHandler

////////////////////////////////////////////////////////////////////////////////

template <class T>
MPI_Datatype DataTypeHandler::GetType ()
{
    static DataTypeRegistrar<T> TheRegistrar;
    return TheRegistrar.GetType ();
}

////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

#endif
