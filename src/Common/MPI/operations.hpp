// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_mpi_operations_hpp
#define CF_Common_mpi_operations_hpp

////////////////////////////////////////////////////////////////////////////////

#include <mpi.h>

#include <Common/MPI/tools.hpp>

////////////////////////////////////////////////////////////////////////////////

/**
  @file operations.hpp
  @author Tamas Banyai
  This header serves the necessary routines to interface C/C++ operationss to MPI types.
  Use the op member fuction of get_mpi_op class to acces MPI_Op;
  An example is provided in this header how to implement a class for custom operations, registration vi MPI_Op_create is automatized.
  If a custom operaton is incompatible with a type, itshould come out compile time.
**/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {
    namespace mpi {

////////////////////////////////////////////////////////////////////////////////

/// @{ ACCESS AND REGISTRATION MECHANISM

namespace detail {

/**
  Support for registering and storing non-built-in operations.
  @returns MPI_Op to the desired operation and type combo.
**/
template<typename T, typename Op> MPI_Op get_mpi_op_impl() {
  static MPI_Op op(MPI_OP_NULL);
  if (op==MPI_OP_NULL) {
    MPI_CHECK_RESULT(MPI_Op_create, (Op::template func<T>, Op::is_commutative, &op));
  }
  return op;
}

} // namespace detail

/**
  Default class to obtain operation.
  @returns MPI_Op to the desired operation and type combo.
**/
template<typename T, typename Op> class get_mpi_op
{
  public:
    /// Accessor member function.
    /// @returns operations in type of MPI_Op
    static MPI_Op op() { return detail::get_mpi_op_impl<T,Op>(); }
};

/// @}

/// @{ BUILT-IN OPERATIONS

class max {};         template<typename T> struct get_mpi_op<T, max>         { public: static MPI_Op op() { return MPI_MAX;  } };
class min {};         template<typename T> struct get_mpi_op<T, min>         { public: static MPI_Op op() { return MPI_MIN;  } };
class plus {};        template<typename T> struct get_mpi_op<T, plus>        { public: static MPI_Op op() { return MPI_SUM;  } };
class multiplies {};  template<typename T> struct get_mpi_op<T, multiplies>  { public: static MPI_Op op() { return MPI_PROD; } };
class logical_and {}; template<typename T> struct get_mpi_op<T, logical_and> { public: static MPI_Op op() { return MPI_LAND; } };
class logical_or {};  template<typename T> struct get_mpi_op<T, logical_or>  { public: static MPI_Op op() { return MPI_LOR;  } };
class logical_xor {}; template<typename T> struct get_mpi_op<T, logical_xor> { public: static MPI_Op op() { return MPI_LXOR; } };
class bitwise_and {}; template<typename T> struct get_mpi_op<T, bitwise_and> { public: static MPI_Op op() { return MPI_BAND; } };
class bitwise_or {};  template<typename T> struct get_mpi_op<T, bitwise_or>  { public: static MPI_Op op() { return MPI_BOR;  } };
class bitwise_xor {}; template<typename T> struct get_mpi_op<T, bitwise_xor> { public: static MPI_Op op() { return MPI_BXOR; } };
// TODO: what are those doing? check and implement
//#define MPI_MAXLOC OMPI_PREDEFINED_GLOBAL(MPI_Op, ompi_mpi_op_maxloc)
//#define MPI_MINLOC OMPI_PREDEFINED_GLOBAL(MPI_Op, ompi_mpi_op_minloc)
//#define MPI_REPLACE OMPI_PREDEFINED_GLOBAL(MPI_Op, ompi_mpi_op_replace)

/// @}

/// @{ EXAMPLE ON HOW TO IMPLEMENT A CUSTOM OPERATION

/**
  Example operator, the only requirement is to have the following two members:
  - a boolean with name is_commutative
  - a function templatized by T respecting
**/
class customplus {
  public:

    /// Static const member variable describing if operation is commutative across processors or not (if not sure, use false).
    static const bool is_commutative=true;

    /// Implementation of the operation. See MPI_Op_create in MPI standard documentation for details.
    template<typename T> static void func(void* in, void* out, int* len, MPI_datatype* type){
      int rank,i;
      T *in_=(T*)in;
      T *out_=(T*)out;
      MPI_Comm_rank(MPI_COMM_WORLD,&rank);
      std::cout << "rank" << rank << "\t: "; for (i=0; i<(const int)(*len); i++) std::cout << " " << in_[i]; std::cout << "\n";
      std::cout << "sum" << rank << "\t: "; for (i=0; i<(const int)(*len); i++) { out_[i]+=in_[i]; std::cout << " " << out_[i]; } std::cout << "\n";
    }
};

/// @}

////////////////////////////////////////////////////////////////////////////////

    } // namespace mpi
  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_mpi_operations_hpp
