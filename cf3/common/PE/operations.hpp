// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_operations_hpp
#define cf3_common_PE_operations_hpp

////////////////////////////////////////////////////////////////////////////////

#include <mpi.h>

#include <boost/type_traits/is_arithmetic.hpp>

#include "common/PE/types.hpp"
// #include "common/PE/debug.hpp" // for debugging mpi

////////////////////////////////////////////////////////////////////////////////

/**
  @file operations.hpp
  @author Tamas Banyai
  This header serves the necessary routines to interface C/C++ operationss to MPI types.
  Use the op member fuction of get_mpi_op class to acces Operation;
  An example is provided in this header how to implement a class for custom operations, registration vi Operation_create is automatized.
  If a custom operaton is incompatible with a type, itshould come out compile time.
**/

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  namespace common {
    namespace PE {

////////////////////////////////////////////////////////////////////////////////

namespace detail {

/**
  Support for registering and storing non-built-in operations.
  @returns Operation to the desired operation and type combo.
**/
template<typename T, typename Op> Operation get_mpi_op_impl() {
  static Operation op((Operation)nullptr);
  if (op==(Operation)nullptr) {
    MPI_CHECK_RESULT(MPI_Op_create, (Op::template func<T>, Op::is_commutative, &op));
  }
  return op;
}

} // namespace detail

/// @{ ACCESS AND REGISTRATION MECHANISM

/**
  Default class to obtain operation.
  @returns Operation to the desired operation and type combo.
**/
template<typename T, typename Op, typename selector = void > class get_mpi_op
{
  public:
    /// Accessor member function.
    /// @returns operations in type of Operation
    static Operation op() { return detail::get_mpi_op_impl<T,Op>(); }
};

/**
  Macro for defining custom operations. It hides defining a class.
  An example for expression defining the plus operation: *out = *out + *in, note that in and out are of type T*.
  @param name is the name of the class
  @param commutative boolean variable describing if operation is commutative across processors or not (if not sure, use false).
  @param expression defining the atomic operation, names in and out are hardcoded.
**/
#define MPI_CUSTOM_OPERATION(name,commutative,expression) class name                             \
  {                                                                                              \
    public:                                                                                      \
      static const bool is_commutative=commutative;                                              \
      template<typename T> static void func(void* in_, void* out_, int* len, Datatype* type) \
      {                                                                                          \
        T *in=(T*)in_;                                                                           \
        T *out=(T*)out_;                                                                         \
        for (int i=0; i<*len; i++)                                                  \
        {                                                                                        \
          expression;                                                                            \
          in++;                                                                                  \
          out++;                                                                                 \
        }                                                                                        \
      }                                                                                          \
  }

/// @}

/// @{ BUILT-IN OPERATIONS
MPI_CUSTOM_OPERATION(max,         true, *out= *in > *out ? *in : *out );
MPI_CUSTOM_OPERATION(min,         true, *out= *in < *out ? *in : *out );
MPI_CUSTOM_OPERATION(plus,        true, *out= *in + *out );
MPI_CUSTOM_OPERATION(multiplies,  true, *out= *in * *out );
MPI_CUSTOM_OPERATION(logical_and, true, *out= *in && *out );
MPI_CUSTOM_OPERATION(logical_or,  true, *out= *in || *out );
MPI_CUSTOM_OPERATION(logical_xor, true, *out= !*in ^ !*out );
MPI_CUSTOM_OPERATION(bitwise_and, true, *out= *in & *out );
MPI_CUSTOM_OPERATION(bitwise_or,  true, *out= *in | *out );
MPI_CUSTOM_OPERATION(bitwise_xor, true, *out= *in ^ *out );

template<typename T> struct get_mpi_op<T, max,         typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_MAX;  } };
template<typename T> struct get_mpi_op<T, min,         typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_MIN;  } };
template<typename T> struct get_mpi_op<T, plus,        typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_SUM;  } };
template<typename T> struct get_mpi_op<T, multiplies,  typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_PROD; } };
template<typename T> struct get_mpi_op<T, logical_and, typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_LAND; } };
template<typename T> struct get_mpi_op<T, logical_or,  typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_LOR;  } };
template<typename T> struct get_mpi_op<T, logical_xor, typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_LXOR; } };
template<typename T> struct get_mpi_op<T, bitwise_and, typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_BAND; } };
template<typename T> struct get_mpi_op<T, bitwise_or,  typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_BOR;  } };
template<typename T> struct get_mpi_op<T, bitwise_xor, typename boost::enable_if<boost::is_arithmetic<T> >::type > { public: static Operation op() { return MPI_BXOR; } };

// TODO: maybe worth implementing those?
//#define MPI_MAXLOC OMPI_PREDEFINED_GLOBAL(Operation, ompi_mpi_op_maxloc)
//#define MPI_MINLOC OMPI_PREDEFINED_GLOBAL(Operation, ompi_mpi_op_minloc)
//#define MPI_REPLACE OMPI_PREDEFINED_GLOBAL(Operation, ompi_mpi_op_replace)

/// @}

/// @{ EXAMPLE ON HOW TO IMPLEMENT A CUSTOM OPERATION

/**
  Example operator, the only requirement is to have the following two members:
  - a boolean with name is_commutative
  - a function templatized by T respecting MPI_User_function syntax
  For ease of use, there is a macro called MPI_CUSTOM_OPERATION.
**/
MPI_CUSTOM_OPERATION(customplus,true,*out=*in+*out);

/// @}

////////////////////////////////////////////////////////////////////////////////

    } // namespace PE
  } // namespace common
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_operations_hpp
