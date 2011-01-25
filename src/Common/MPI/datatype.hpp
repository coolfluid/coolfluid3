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
  It is basically a reduced version of boost.mpi.
  Use the function get_mpi_datatype to acces MPI_type;
  New plain old data types are automatically registered.
  Non built-in types with smaller sizes result to lower performance due to memcpy.
  Never-ever use MPI_Datatype outside of namespace CF:Common::mpi.
  If you really need to use it, than rather extend the interface.
**/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
    namespace mpi {

////////////////////////////////////////////////////////////////////////////////

/// @{ ACCESS AND REGISTRATION MECHANISM

/// Default of get_mpi_datatype_impl which returns nullptr.
namespace detail {
template <typename T> inline MPI_Datatype get_mpi_datatype_impl(const T& ) { return nullptr; };
} // namespace detail

/// Function to obtain MPI_Datatype from type or if non-existent than commit one to MPI and store it.
template <typename T> inline MPI_Datatype get_mpi_datatype(const T& ref_of_type)
{
  if (detail::get_mpi_datatype_impl(ref_of_type)!=nullptr) return detail::get_mpi_datatype_impl<T>(ref_of_type);
  static MPI_Datatype type(nullptr);
  if (type==nullptr){
    //PEProcessSortedExecute(mpi::PE::instance(),-1,CFinfo << "Registering type of size " << sizeof(T) << CFendl;);
    if (!boost::is_pod<T>::value) throw NotSupported(FromHere(),"Non-POD (plain old datatype) is not supported by parallel environment communications.");
    MPI_CHECK_RESULT(MPI_Type_contiguous,(sizeof(T), MPI_BYTE, &type));
    MPI_CHECK_RESULT(MPI_Type_commit,(&type));
  }
  return type;
}

/// @}

/// @{ DEFAULT DATATYPE WRAPPERS

/// Template specializations for built-in types.
namespace detail {
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
} // namespace detail


/// @}

////////////////////////////////////////////////////////////////////////////////

    } // namespace mpi
  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_mpi_datatype_hpp
