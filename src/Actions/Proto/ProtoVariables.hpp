// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_ProtoVariables_hpp
#define CF_Actions_ProtoVariables_hpp

#include <boost/mpl/int.hpp>

#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/ElementType.hpp"

#include "Actions/Proto/ProtoDomain.hpp"
#include <boost/concept_check.hpp>

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
  
  template<typename T1>
  Var(T1& par1) : T(par1) {}
  
  template<typename T1, typename T2>
  Var(const T1& par1, const T2& par2) : T(par1, par2) {}
  
  template<typename T1, typename T2>
  Var(T1& par1, T2& par2) : T(par1, par2) {}
};

/// Represent const element nodes. Using this in an expression is like passing the nodes of the current element
struct ConstNodes
{
  ConstNodes()
  {
  }
  
  ConstNodes(Mesh::CRegion::Ptr r) : region(r)
  {
  }
  
  /// Get the element type, based on the CElements currently traversed.
  const Mesh::ElementType& element_type(const Mesh::CElements& elements) const
  {
    return elements.element_type();
  }
  
  /// Root region with which the nodes are associated
  Mesh::CRegion::Ptr region;
};

/// Mutable node field data
template<typename T>
struct Field
{
  Field() : field_name() {}
  
  Field(const std::string& field_nm, const std::string var_nm) : field_name(field_nm), var_name(var_nm) {}
  
  /// Get the element type, based on the CElements currently traversed.
  const Mesh::ElementType& element_type(const Mesh::CElements& elements) const
  {
    return elements.get_field_elements(field_name).element_type();
  }
  
  std::string field_name;
  std::string var_name;
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
  
  template<typename T1>
  MeshTerm(T1& par1) : base_type(boost::proto::make_expr<boost::proto::tag::terminal, MeshDomain>(Var<boost::mpl::int_<I>, T>(par1))) {}
  
  template<typename T1, typename T2>
  MeshTerm(const T1& par1, const T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal, MeshDomain>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}
  
  template<typename T1, typename T2>
  MeshTerm(T1& par1, T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal, MeshDomain>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}
  
  BOOST_PROTO_EXTENDS_USING_ASSIGN(MeshTerm)
};

/// Encapsulate a system matrix, stored as dense Eigen matrix
template< typename MatrixT >
struct EigenDenseMatrix
{
  EigenDenseMatrix() : m_matrix(0) {}
  
  EigenDenseMatrix(MatrixT& m)
  {
    set_matrix(m);
  }
  
  MatrixT& matrix()
  {
    return *m_matrix;
  }
  
  void set_matrix(MatrixT& m)
  {
    m_matrix = &m;
  }
  
private:
  MatrixT* m_matrix;
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
struct EigenDenseDirichletBC
{
  EigenDenseDirichletBC() : m_matrix(0), m_rhs(0)
  { 
  }
  
  EigenDenseDirichletBC(MatrixT& m, RhsT& r)
  {
    set_matrix(m);
    set_rhs(r);
  }
  
  MatrixT& matrix()
  {
    return *m_matrix;
  }
  
  RhsT& rhs()
  {
    return *m_rhs;
  }
  
  void set_matrix(MatrixT& m)
  {
    m_matrix = &m;
  }
  
  void set_rhs(RhsT& r)
  {
    m_rhs = &r;
  }
  
private:
  MatrixT* m_matrix;
  RhsT* m_rhs;
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

} // Proto
} // Actions
} // CF

#endif // CF_Actions_ProtoVariables_hpp
