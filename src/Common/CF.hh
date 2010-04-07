#ifndef CF_hh
#define CF_hh

//////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_CONFIG_H
#  include "coolfluid_config.h"
#endif // CF_HAVE_CONFIG_H

//////////////////////////////////////////////////////////////////////////////

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

#include "Common/CommonAPI.hh"
#include "Common/AssertionManager.hh"
#include "Common/DemangledTypeID.hh"
#include "Common/PtrAlloc.hh"

//////////////////////////////////////////////////////////////////////////////

/// Definition of CF namespace.
/// @author Tiago Quintino
/// @author Andrea Lani
namespace CF {

  /// Definition of the basic types for possible portability conflicts

  /// typedef for unsigned int
  typedef unsigned int Uint;

  /// Definition of the default precision
  typedef CF_REAL_TYPE Real;

  // if nothing defined
  #if !defined CF_PRECISION_DOUBLE && !defined CF_PRECISION_SINGLE && !defined CF_PRECISION_LONG_DOUBLE
    typedef double Real;
  #endif

typedef std::complex< Real >  Complex;

//////////////////////////////////////////////////////////////////////////////

  /// Enumeration of the dimensions
  enum Dim                  { DIM_0D, DIM_1D, DIM_2D, DIM_3D };
  /// Enumeration of the coordinates indexes
  enum CoordXYZ             { XX, YY, ZZ };
  /// Enumeration of the reference coordinates indexes
  enum CoordRef             { KSI, ETA, ZTA };

//////////////////////////////////////////////////////////////////////////////

} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_hh
