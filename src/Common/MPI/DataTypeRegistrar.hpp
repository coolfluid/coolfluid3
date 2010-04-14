#ifndef DataTypeREGISTRAR_HH
#define DataTypeREGISTRAR_HH

#include <mpi.h>

#include <typeinfo>

#include "Common/Log.hpp"

//#ifndef CFinfo
// #error CFinfo is defined
//#else
// #error CFinfo is *not* defined
//#endif

/*
* UNSAFE_MPITYPES enables generic MPI Type registration code.
* This means MPI no longer 'knows' the exact meaning of the data,
* but will consider it as meaningless bytes.
*
* This shouldn't be a problem when staying within one architecture within
* the cluster...
*

#define UNSAFE_MPITYPES

#ifdef UNSAFE_MPITYPES
#include "Common/MPI/DataTypeRegistrar_Helper.hpp"
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace MPI  {

////////////////////////////////////////////////////////////////////////////////

#ifndef UNSAFE_MPITYPES
//=========================== SAFE MPI Types ==========================
/*template <class T>
class DataTypeRegistrar
{
public:

    // Every type should override this
    MPI_Datatype GetType () const
        { cf_assert (false); };

};*/

//===================================================================
#else
//=========================== UNSAFE MPI Types ======================

template <class T>
class DataTypeRegistrar : public DataTypeRegistrar_Helper {
public:

    DataTypeRegistrar()
    {
      DoRegister ();
    }

    virtual ~DataTypeRegistrar ()
    {
    }

    virtual void Register (MPI_Comm Comm)
    {
      //CFinfo << "DataTypeRegistrar(generic): Registering type " <<
//        typeid(T).name() << " of size " << sizeof (T) <<
//        '\n';
      MPI_Type_contiguous (sizeof (T), MPI_BYTE, &TheType);
      MPI_Type_commit (&TheType);
    }

    virtual void UnRegister ()
    {
      //CFLogDebug( "DataTypeRegistrar(generic): "
//        << "DeRegistering type " <<
//        typeid(T).name() << " of size " << sizeof (T) << "\n");
      MPI_Type_free (&TheType);
    }

}; // end class DataTypeRegistrar

#endif // UNSAFE_MPITYPES

#define REGISTRAR_TYPE(CT,MT) \
    template <>           \
    class DataTypeRegistrar<CT>      \
    {             \
  public:           \
      MPI_Datatype GetType () const   \
    { return MT; };       \
    };


REGISTRAR_TYPE(double, MPI_DOUBLE)
REGISTRAR_TYPE(float,MPI_FLOAT)
REGISTRAR_TYPE(int,MPI_INT)
REGISTRAR_TYPE(unsigned int,MPI_UNSIGNED)
REGISTRAR_TYPE(long, MPI_LONG)
REGISTRAR_TYPE(unsigned long, MPI_LONG)

////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

#endif
