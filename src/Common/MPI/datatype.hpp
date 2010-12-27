// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_mpi_datatype_hpp
#define CF_Common_mpi_datatype_hpp

////////////////////////////////////////////////////////////////////////////////

#include <mpi.h>

#include <boost/type_traits/is_pod.hpp>

#include <Common/MPI/tools.hpp>

////////////////////////////////////////////////////////////////////////////////

/**
  @file datatype.hpp
  @author Tamas Banyai
  This header serves the necessary routines to interface C/C++ datatypes to MPI types.
  Use the function get_mpi_datatype to acces MPI_type;
  New plain old data types are automatically registered.
  Non built-in types results to somewhat lower performance do to a need for loookup.
**/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
    namespace mpi {

////////////////////////////////////////////////////////////////////////////////

/// @{ ACCESS AND REGISTRATION MECHANISM

/// Default of get_mpi_datatype_impl which returns nullptr.
template <typename T> inline MPI_Datatype get_mpi_datatype_impl(const T& ) { return nullptr; };

/// Function for checking if there is an mpi datatype associated to the template type or not.
template <typename T> inline bool is_mpi_datatype(const T& ref_of_type) { return get_mpi_datatype_impl(ref_of_type)!=nullptr; };

/// Function to obtain MPI_Datatype from type
template <typename T> inline MPI_Datatype get_mpi_datatype(const T& ref_of_type)
{

}

/// @}

/// @{ DEFAULT DATATYPES

/// Template specializations for built-in types.
template<> inline MPI_Datatype get_mpi_datatype_impl <char>               (const char& )               { return MPI_CHAR; };
template<> inline MPI_Datatype get_mpi_datatype_impl <unsigned char>      (const unsigned char& )      { return MPI_UNSIGNED_CHAR; };
template<> inline MPI_Datatype get_mpi_datatype_impl <short>              (const short& )              { return MPI_SHORT; };
template<> inline MPI_Datatype get_mpi_datatype_impl <unsigned short>     (const unsigned short& )     { return MPI_UNSIGNED_SHORT; };
template<> inline MPI_Datatype get_mpi_datatype_impl <int>                (const int& )                { return MPI_INT; };
template<> inline MPI_Datatype get_mpi_datatype_impl <unsigned int>       (const unsigned int& )       { return MPI_UNSIGNED; }; // there is no MPI_UNSIGNED_INT
template<> inline MPI_Datatype get_mpi_datatype_impl <long>               (const long& )               { return MPI_LONG; };
template<> inline MPI_Datatype get_mpi_datatype_impl <unsigned long>      (const unsigned long& )      { return MPI_UNSIGNED_LONG; };
template<> inline MPI_Datatype get_mpi_datatype_impl <long long>          (const long long& )          { return MPI_LONG_LONG; };
template<> inline MPI_Datatype get_mpi_datatype_impl <unsigned long long> (const unsigned long long& ) { return MPI_UNSIGNED_LONG_LONG; };
template<> inline MPI_Datatype get_mpi_datatype_impl <float>              (const float&)               { return MPI_FLOAT; };
template<> inline MPI_Datatype get_mpi_datatype_impl <double>             (const double& )             { return MPI_DOUBLE; };
template<> inline MPI_Datatype get_mpi_datatype_impl <long double>        (const long double& )        { return MPI_LONG_DOUBLE; };

/// @}

////////////////////////////////////////////////////////////////////////////////

    } // namespace mpi
  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_mpi_datatype_hpp

/*
// Copyright 2004 The Trustees of Indiana University.
// Copyright 2005 Matthias Troyer.
// Copyright 2006 Douglas Gregor <doug.gregor -at- gmail.com>.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
//           Matthias Troyer

#ifndef CF_Common_mpi_datatype_hpp
#define CF_Common_mpi_datatype_hpp

#include <mpi.h>
#include <boost/config.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpi/detail/mpi_datatype_cache.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/archive/basic_archive.hpp>
#include <boost/serialization/item_version_type.hpp>
#include <utility> // for std::pair

namespace boost { namespace mpi {

template<typename T> MPI_Datatype get_mpi_datatype(const T& x)
{
  BOOST_MPL_ASSERT((is_mpi_datatype<T>));
  return detail::mpi_datatype_cache().datatype(x);
}

// Don't parse this part when we're generating Doxygen documentation.
#ifndef BOOST_MPI_DOXYGEN

/// INTERNAL ONLY
#define BOOST_MPI_DATATYPE(CppType, MPIType, Kind)                      \
template<>                                                              \
inline MPI_Datatype                                                     \
get_mpi_datatype< CppType >(const CppType&) { return MPIType; }         \
                                                                        \
template<>                                                              \
 struct BOOST_JOIN(is_mpi_,BOOST_JOIN(Kind,_datatype))< CppType >       \
: boost::mpl::bool_<true>                                               \
{}



/// specialization of is_mpi_datatype for pairs
template <class T, class U>
struct is_mpi_datatype<std::pair<T,U> >
 : public mpl::and_<is_mpi_datatype<T>,is_mpi_datatype<U> >
{
};

// Define wchar_t specialization of is_mpi_datatype, if possible.
#if !defined(BOOST_NO_INTRINSIC_WCHAR_T) && \
  (defined(MPI_WCHAR) || (defined(MPI_VERSION) && MPI_VERSION >= 2))
BOOST_MPI_DATATYPE(wchar_t, MPI_WCHAR, builtin);
#endif

// Define signed char specialization of is_mpi_datatype, if possible.
#if defined(MPI_SIGNED_CHAR) || (defined(MPI_VERSION) && MPI_VERSION >= 2)
BOOST_MPI_DATATYPE(signed char, MPI_SIGNED_CHAR, builtin);
#endif


#endif // Doxygen

namespace detail {
  inline MPI_Datatype build_mpi_datatype_for_bool()
  {
    MPI_Datatype type;
    MPI_Type_contiguous(sizeof(bool), MPI_BYTE, &type);
    MPI_Type_commit(&type);
    return type;
  }
}

/// Support for bool. There is no corresponding MPI_BOOL.
/// INTERNAL ONLY
template<>
inline MPI_Datatype get_mpi_datatype<bool>(const bool&)
{
  static MPI_Datatype type = detail::build_mpi_datatype_for_bool();
  return type;
}

/// INTERNAL ONLY
template<>
struct is_mpi_datatype<bool>
  : boost::mpl::bool_<true>
{};



} } // end namespace boost::mpi

// define a macro to make explicit designation of this more transparent
#define BOOST_IS_MPI_DATATYPE(T)              \
namespace boost {                             \
namespace mpi {                               \
template<>                                    \
struct is_mpi_datatype< T > : mpl::true_ {};  \
}}                                            \
/**/


//#endif // BOOST_MPI_MPI_DATATYPE_HPP
