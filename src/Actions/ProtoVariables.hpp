// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoVariables_hpp
#define CF_Actions_ProtoVariables_hpp

#include <boost/mpl/int.hpp>

#include "Actions/ProtoDomain.hpp"

#include "Math/RealMatrix.hpp"
#include "Math/RealVector.hpp"

namespace CF {
namespace Actions {

// Placeholders for different types of data
struct ElementIdxHolder {};
boost::proto::terminal<ElementIdxHolder>::type const _elem = {{}}; // Represents an element index ({{{}}} makes this a static initialization)

struct ElementNodeIdxHolder {};
boost::proto::terminal<ElementNodeIdxHolder>::type const _elem_node = {{}}; // Represents a node index relative to the element

struct MappedCoordHolder {};
boost::proto::terminal<MappedCoordHolder>::type const _mapped_coord = {{}};

/// Creates a variable that has unique ID I
template<typename I, typename T>
struct Var : T
{
  /// Type that is wrapped
  typedef T type;
  Var() : T() {}
  
  /// Index of the var
  typedef I index_type;
  
  template<typename T1>
  Var(const T1& par1) : T(par1) {}
  
  template<typename T1, typename T2>
  Var(const T1& par1, const T2& par2) : T(par1, par2) {}
};

/// Represent const element nodes. This will be replaced with the current list of element nodes
struct ConstNodes
{
};

std::ostream& operator<<(std::ostream& output, const ConstNodes&)
{
  output << "ConstNodes";
  return output;
}

/// Const node field data
template<typename T>
struct ConstField
{
  ConstField() : field_name() {}
  
  ConstField(const std::string& fld_name) : field_name(fld_name) {}
  
  std::string field_name;
};

/// Mutable node field data
template<typename T>
struct Field
{
  Field() : field_name() {}
  
  Field(const std::string& field_nm, const std::string var_nm) : field_name(field_nm), var_name(var_nm) {}
  
  std::string field_name;
  std::string var_name;
};

template<typename T>
std::ostream& operator<<(std::ostream& output, const ConstField<T>& fd)
{
  output << "field " << fd.field_name;
  return output;
}

/// Represent mutable element nodes
struct MutableNodes
{
};

template<Uint I, typename T>
struct MeshTerm : boost::proto::result_of::make_expr
                  <
                    boost::proto::tag::terminal
                  , MeshDomain
                  , Var<boost::mpl::int_<I>, T>
                  >::type
{
  typedef typename boost::proto::result_of::make_expr
  <
    boost::proto::tag::terminal
  , MeshDomain
  , Var<boost::mpl::int_<I>, T>
  >::type base_type;
  
  MeshTerm() : base_type(boost::proto::make_expr<boost::proto::tag::terminal, MeshDomain>(Var<boost::mpl::int_<I>, T>())) {}
  
  template<typename T1>
  MeshTerm(const T1& par1) : base_type(boost::proto::make_expr<boost::proto::tag::terminal, MeshDomain>(Var<boost::mpl::int_<I>, T>(par1))) {}
  
  template<typename T1, typename T2>
  MeshTerm(const T1& par1, const T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal, MeshDomain>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}
  
  BOOST_PROTO_EXTENDS_USING_ASSIGN(MeshTerm)
};

///////////////////////////////////////////////////////////////////
// Expression result size and type

/// Storage for result size and type
template<Uint NbRows, Uint NbCols>
struct ResultSize
{
  /// Integral constant representing the number of rows in the result
  typedef boost::mpl::int_<NbRows> nb_rows_c;
  
  /// Integral constant representing the number of columns in the result
  typedef boost::mpl::int_<NbCols> nb_cols_c;
  
  /// Result type that takes the given sizes
  typedef RealMatrix result_type;
  
  /// Initialize the result
  static void init(result_type& result)
  {
    result.resize(nb_rows_c::value, nb_cols_c::value);
  }
  
  ResultSize<NbRows, NbCols>
  operator+=(const ResultSize<NbRows, NbCols>)
  {
    return *this;
  }
};

template<>
struct ResultSize< 1, 1 >;

/// Specialization for column vectors
template<Uint NbRows>
struct ResultSize< NbRows, 1 >
{
  /// Integral constant representing the number of rows in the result
  typedef boost::mpl::int_<NbRows> nb_rows_c;
  
  /// Integral constant representing the number of columns in the result
  typedef boost::mpl::int_<1> nb_cols_c;
  
  /// Result type that takes the given sizes
  typedef RealVector result_type;
  
  /// Initialize the result
  static void init(result_type& result)
  {
    result.resize(nb_rows_c::value);
  }
  
  ResultSize<NbRows, 1>
  operator+=(const ResultSize<NbRows, 1>)
  {
    return *this;
  }
  
  ResultSize<1, 1>
  operator[](const Uint) const;
};

/// Specialization for scalars
template<>
struct ResultSize< 1, 1 >
{
  /// Integral constant representing the number of rows in the result
  typedef boost::mpl::int_<1> nb_rows_c;
  
  /// Integral constant representing the number of columns in the result
  typedef boost::mpl::int_<1> nb_cols_c;
  
  /// Result type that takes the given sizes
  typedef Real result_type;
  
  /// Initialize the result
  static void init(result_type& result)
  {
    result = 0.;
  }
  
  template<Uint NbRows, Uint NbCols>
  const ResultSize<NbRows, NbCols>
  operator+=(const ResultSize<NbRows, NbCols> other)
  {
    return other;
  }
};

typedef ResultSize< 1 , 1  > ScalarT;

template<Uint NbRows>
ResultSize< 1, 1 > ResultSize< NbRows, 1  >::operator[] ( const CF::Uint ) const
{
  return ResultSize<1, 1>();
}

/// Sum doesn't change anything
template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator+(const ResultSize<NbRows,NbCols>& x, const ResultSize<NbRows,NbCols> y)
{
  return ResultSize<NbRows,NbCols>();
}

/// Overloads for Reals
template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator+(const ResultSize<NbRows,NbCols>& x, const Real& y)
{
  return ResultSize<NbRows,NbCols>();
}

template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator+(const Real& x, const ResultSize<NbRows,NbCols> y)
{
  return ResultSize<NbRows,NbCols>();
}

/// Subtraction
template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator-(const ResultSize<NbRows,NbCols>& x, const ResultSize<NbRows,NbCols> y)
{
  return ResultSize<NbRows,NbCols>();
}

/// Overloads for Reals
template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator-(const ResultSize<NbRows,NbCols>& x, const Real& y)
{
  return ResultSize<NbRows,NbCols>();
}

template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator-(const Real& x, const ResultSize<NbRows,NbCols> y)
{
  return ResultSize<NbRows,NbCols>();
}

/// Multiplication and division by a constant
template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator*(const ResultSize<NbRows,NbCols>& x, const Real& y)
{
  return ResultSize<NbRows,NbCols>();
}

template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator*(const Real& x, const ResultSize<NbRows,NbCols> y)
{
  return ResultSize<NbRows,NbCols>();
}

template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator/(const ResultSize<NbRows,NbCols>& x, const Real& y)
{
  return ResultSize<NbRows,NbCols>();
}

template <Uint NbRows, Uint NbCols>
ResultSize<NbRows,NbCols>
operator/(const ResultSize<NbRows,NbCols>& x, const ScalarT& y)
{
  return ResultSize<NbRows,NbCols>();
}

/// Types of product
template <Uint NbRows1, Uint NbCols1, Uint NbRows2, Uint NbCols2>
struct ProductType; // generic case is an error

/// Matrix * matrix
template<Uint NbRows, Uint CommonNb, Uint NbCols>
struct ProductType<NbRows, CommonNb, CommonNb, NbCols>
{
  typedef ResultSize<NbRows, NbCols> type;
};

/// Scalar * matrix
template<Uint NbRows, Uint NbCols>
struct ProductType<1, 1, NbRows, NbCols>
{
  typedef ResultSize<NbRows, NbCols> type;
};

/// Matrix * scalar
template<Uint NbRows, Uint NbCols>
struct ProductType<NbRows, NbCols, 1, 1>
{
  typedef ResultSize<NbRows, NbCols> type;
};

/// Scalar * matrix
template<Uint NbCols>
struct ProductType<1, 1, 1, NbCols>
{
  typedef ResultSize<1, NbCols> type;
};

/// Scalar*Scalar
template<>
struct ProductType<1, 1, 1, 1>
{
  typedef ScalarT type;
};

template <Uint NbRows1, Uint NbCols1, Uint NbRows2, Uint NbCols2>
typename ProductType<NbRows1, NbCols1, NbRows2, NbCols2>::type
operator*(const ResultSize<NbRows1, NbCols1>&, const ResultSize<NbRows2, NbCols2>)
{
  return typename ProductType<NbRows1, NbCols1, NbRows2, NbCols2>::type();
}

} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoVariables_hpp
