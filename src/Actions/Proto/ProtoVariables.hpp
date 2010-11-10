// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoVariables_hpp
#define CF_Actions_ProtoVariables_hpp

#include <boost/mpl/int.hpp>

#include "Actions/Proto/ProtoDomain.hpp"


namespace CF {
namespace Actions {
namespace Proto {

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

/// Encapsulae a matrix
template< typename MatrixT >
struct MatrixView
{
  MatrixView(MatrixT& m) : matrix(m) {}
  MatrixT& matrix;
};

/// System matrix terminal. Use assignment operators to assemble by element
template< typename MatrixT >
struct BlockMatrix
  : MeshExpr< typename boost::proto::terminal< MatrixView<MatrixT> >::type >
{
    typedef typename boost::proto::terminal< MatrixView<MatrixT> >::type expr_type;

    BlockMatrix(MatrixT& m)
      : MeshExpr<expr_type>( expr_type::make( m ) )
    {}
    
    BOOST_PROTO_EXTENDS_USING_ASSIGN(BlockMatrix)
};

/// Helper struct for assignment to a matrix
template<typename OpTagT>
struct BlockAssignmentOp;

#define MAKE_ASSIGN_OP(__tagname, __op)    \
template<> \
struct BlockAssignmentOp<__tagname> \
{ \
  template<typename MatrixT, int NbRows, int NbCols, typename ConnectivityT> \
  static void assign(MatrixT& matrix, const Eigen::Matrix<Real, NbRows, NbCols>& rhs, const ConnectivityT& connectivity) \
  { \
    for(Uint i = 0; i != NbRows; ++i) \
      for(Uint j = 0; j != NbCols; ++j) \
        matrix(connectivity[i], connectivity[j]) __op rhs(i, j); \
  } \
};

MAKE_ASSIGN_OP(boost::proto::tag::assign, =)
MAKE_ASSIGN_OP(boost::proto::tag::plus_assign, +=)

#undef MAKE_ASSIGN_OP

/// Encapsulate a system matrix and a RHS
template<typename MatrixT, typename RhsT>
struct DirichletBCView
{
  DirichletBCView(MatrixT& m, RhsT& r) : matrix(m), rhs(r)
  {
  }
    
  MatrixT& matrix;
  RhsT& rhs;
};

/// Dirichlet boundary conditions terminal. Use the assignment operator to set the BC in a node
template< typename MatrixT, typename RhsT >
struct DirichletBC
  : MeshExpr< typename boost::proto::terminal< DirichletBCView<MatrixT, RhsT> >::type >
{
    typedef typename boost::proto::terminal< DirichletBCView<MatrixT, RhsT> >::type expr_type;

    DirichletBC(MatrixT& m, RhsT& rhs)
      : MeshExpr<expr_type>( expr_type::make( DirichletBCView<MatrixT, RhsT>(m, rhs) ) )
    {}
    
    BOOST_PROTO_EXTENDS_USING_ASSIGN(DirichletBC)
};

template<typename MatrixT, typename RhsT>
void assign_dirichlet(MatrixT& matrix, RhsT& rhs, const Real value, const Uint node_idx)
{
  const int size = matrix.rows();
  cf_assert(matrix.cols() == size);
  cf_assert(rhs.rows() == size);
  
  matrix.row(node_idx).setZero();
  matrix(node_idx, node_idx) = 1.;
  rhs[node_idx] = value;
}

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_ProtoVariables_hpp
