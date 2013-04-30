// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file CF.hpp
/// @brief coolfluid3 header, included almost everywhere

#ifndef cf3_hpp
#define cf3_hpp

#include <cstddef> // For ptrdiff_t in GCC 4.6
#include <string>
#include <vector>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>

#include "coolfluid-config.hpp"  // coolfluid system configuration

////////////////////////////////////////////////////////////////////////////////

/// Define the macros:
///    CF3_LOCAL_API
///    CF3_IMPORT_API
///    CF3_EXPORT_API

// Define or not the extern keyword for templates
#ifdef CF3_HAVE_EXTERN_TEMPLATES
#   define CF3_TEMPLATE_EXTERN extern
#else
#   define CF3_TEMPLATE_EXTERN
#endif

// Visual Studio :
//    all symbols are local (invisible) by default
#  if defined (_MSC_VER)
#    define CF3_LOCAL_API
#    define CF3_IMPORT_API    __declspec(dllimport)
#    define CF3_EXPORT_API    __declspec(dllexport)
#  endif

// GNU Compiler :  all symbols are global (visible) by default
#  if defined (__GNUC__) // && defined (__unix__)
#    define CF3_LOCAL_API    __attribute__ ((visibility("hidden")))
#    define CF3_EXPORT_API   __attribute__ ((visibility("default")))
#    define CF3_IMPORT_API   __attribute__ ((visibility("default")))
#  endif

// For unrecognized compilers
#  ifndef CF3_LOCAL_API
#    if defined ( CF3_ACCEPT_ANY_COMPILER )
#      define CF3_LOCAL_API
#      define CF3_EXPORT_API
#      define CF3_IMPORT_API
#    else
#      error "Unrecognised compiler and / or platform"
#    endif
#  endif

////////////////////////////////////////////////////////////////////////////////

// define the nullptr either as macro or as nullptr idiom until C++0x
#ifdef CF3_CXX_SUPPORTS_NULLPTR
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

/// @brief Top-level namespace for coolfluid
///
/// This namespace holds all coolfluid related namespaces,
/// classes, functions.
namespace cf3 {

/// typedef for unsigned int
typedef unsigned int Uint;

/// Definition of the default precision
#ifdef CF3_REAL_IS_FLOAT
typedef float Real;
#endif
#ifdef CF3_REAL_IS_DOUBLE
typedef double Real;
#endif
#ifdef CF3_REAL_IS_LONGDOUBLE
typedef long double Real;
#endif

////////////////////////////////////////////////////////////////////////////////

/// @brief Deletes a pointer and makes sure it is set to nullptr afterwards
///
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

} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_hpp
