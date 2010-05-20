#ifndef CF_hpp
#define CF_hpp

////////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_CONFIG_H
#  include "coolfluid_config.h"
#endif // CF_HAVE_CONFIG_H

////////////////////////////////////////////////////////////////////////////////

#include <memory>    // for std::auto_ptr
#include <string>    // for std::string
#include <sstream>   // string stream
#include <vector>    // for std::vector
#include <map>       // for std::map
#include <cmath>     // all sorts of mathematical functions and constants
#include <algorithm> // all sorts of algorithms on stl containers
#include <utility>   // for stl::pair
#include <typeinfo>  // for typeid
#include <complex>   // for complex numbers and complex functions

#include <boost/checked_delete.hpp>    // for boost::checked_delete

#include "Common/CommonAPI.hpp"
#include "Common/AssertionManager.hpp"
#include "Common/DemangledTypeID.hpp"

////////////////////////////////////////////////////////////////////////////////

/// CF namespace contains some generic typedefs and helper functions
/// @author Tiago Quintino
/// @author Andrea Lani
namespace CF {

// Definition of the basic types for possible portability conflicts

/// typedef for unsigned int
typedef unsigned int Uint;

/// Definition of the default precision
typedef CF_REAL_TYPE Real;

// if nothing defined
#if !defined CF_PRECISION_DOUBLE && !defined CF_PRECISION_SINGLE && !defined CF_PRECISION_LONG_DOUBLE
    typedef double Real;
#endif

typedef std::complex< Real >  Complex;

////////////////////////////////////////////////////////////////////////////////

/// Enumeration of the dimensions
enum Dim                  { DIM_0D, DIM_1D, DIM_2D, DIM_3D };
/// Enumeration of the coordinates indexes
enum CoordXYZ             { XX, YY, ZZ };
/// Enumeration of the reference coordinates indexes
enum CoordRef             { KSI, ETA, ZTA };

////////////////////////////////////////////////////////////////////////////////

///  Definition of CFNULL
#define CFNULL 0

/// @brief Deletes a pointer and makes sure it is set to CFNULL afterwards
/// It would not have to check for CFNULL before deletion, as
/// deleting a null is explicitely allowed by the standard.
/// Nevertheless it does check, to avoid problems with not so compliant compilers.
/// Do not use this function with data allocate with new [].
/// @author Tiago Quintino
/// @pre ptr has been allocated with operator new
/// @param ptr pointer to be deleted
/// @post  ptr equals CFNULL
template <class TYPE>
    void delete_ptr (TYPE*& ptr)
{
  if (ptr != CFNULL)
  {
    boost::checked_delete( ptr );
    ptr = CFNULL;
  }
}

/// @brief Deletes a pointer and makes sure it is set to CFNULL afterwards
/// It would not have to check for CFNULL before deletion, as
/// deleting a null is explicitely allowed by the standard.
/// Nevertheless it does check, to avoid problems with not so compliant compilers.
/// Do not use this function with data allocate with new.
/// @author Tiago Quintino
/// @pre ptr has been allocated with operator new []
/// @param ptr pointer to be deleted
/// @post  ptr equals CFNULL
template <class TYPE>
    void delete_ptr_array (TYPE*& ptr)
{
  if (ptr != CFNULL)
  {
    boost::checked_array_delete( ptr );
    ptr = CFNULL;
  }
}

/// functor for deleting objects by calling the safer delete_ptr function
template < typename BASE >
    struct Deleter { void operator()( BASE * p ) { delete_ptr<BASE>(p); } };

////////////////////////////////////////////////////////////////////////////////

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_hpp
