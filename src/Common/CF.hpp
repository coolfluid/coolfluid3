// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_hpp
#define CF_hpp

////////////////////////////////////////////////////////////////////////////////

#include "coolfluid_config.h"

////////////////////////////////////////////////////////////////////////////////

#include <memory>    // for std::auto_ptr
#include <string>    // for std::string
#include <vector>    // for std::vector
#include <map>       // for std::map
#include <algorithm> // all sorts of algorithms on stl containers
#include <utility>   // for stl::pair
#include <typeinfo>  // for typeid

#include <boost/utility.hpp>           // for boost::noncopyable
#include <boost/checked_delete.hpp>    // for boost::checked_delete

#include "Common/CommonAPI.hpp"
#include "Common/AssertionManager.hpp"
#include "Common/BoostAssert.hpp"
#include "Common/TypeInfo.hpp"

////////////////////////////////////////////////////////////////////////////////

// define the nullptr either as macro or as nullptr idiom until C++0x
#ifdef CF_CXX_SUPPORTS_NULLPTR
const class nullptr_t
{
public:
  template<class T> operator T*() const { return 0; }
  template<class C, class T> operator T C::*() const { return 0; }
private:
  void operator&() const;
} nullptr = {};
#else
  #define nullptr 0
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

/// typedef for unsigned int
typedef unsigned int Uint;

/// Definition of the default precision
#ifdef CF_REAL_IS_FLOAT
typedef float Real;
#endif
#ifdef CF_REAL_IS_DOUBLE
typedef double Real;
#endif
#ifdef CF_REAL_IS_LONGDOUBLE
typedef long double Real;
#endif

////////////////////////////////////////////////////////////////////////////////

/// @brief Deletes a pointer and makes sure it is set to nullptr afterwards
/// It would not have to check for nullptr before deletion, as
/// deleting a null is explicitely allowed by the standard.
/// Nevertheless it does check, to avoid problems with not so compliant compilers.
/// Do not use this function with data allocate with new [].
/// @author Tiago Quintino
/// @pre ptr has been allocated with operator new
/// @param ptr pointer to be deleted
/// @post  ptr equals nullptr
template <class TYPE>
  void delete_ptr (TYPE*& ptr)
{
  if (ptr != nullptr)
  {
    boost::checked_delete( ptr );
    ptr = nullptr;
  }
}

/// @brief Deletes a pointer to array and makes sure it is set to nullptr afterwards
/// @see delete_ptr
/// @author Tiago Quintino
/// @pre ptr has been allocated with operator new []
/// @param ptr pointer to be deleted
/// @post  ptr equals nullptr
template <class TYPE>
  void delete_ptr_array (TYPE*& ptr)
{
  if (ptr != nullptr)
  {
    boost::checked_array_delete( ptr );
    ptr = nullptr;
  }
}

/// functor for deleting objects by calling the safer delete_ptr function
template < typename BASE >
struct Deleter { void operator()( BASE * p ) { delete_ptr<BASE>(p); } };

/// predicate for comparison to nullptr
template < typename T >
bool is_not_null ( T ptr ) { return ptr != nullptr; }

/// predicate for comparison to nullptr
template < typename T >
bool is_null ( T ptr ) { return ptr == nullptr; }

////////////////////////////////////////////////////////////////////////////////

/// Derive from this class if you want a class that is not instantiable.
template < class TYPE >
class NonInstantiable {
private:
    NonInstantiable ();
};

////////////////////////////////////////////////////////////////////////////////

} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_hpp
