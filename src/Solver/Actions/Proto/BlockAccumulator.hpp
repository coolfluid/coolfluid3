// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Solver_Actions_Proto_BlockAccumulator_hpp
#define CF_Solver_Actions_Proto_BlockAccumulator_hpp

#include <boost/proto/core.hpp>

#include "Math/MatrixTypes.hpp"

#include "Solver/CEigenLSS.hpp"

/// @file
/// System matrix block accumultation. Current prototype uses dense a dense Eigen matrix and is purely for proof-of-concept

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Tag for system matrix
struct SystemMatrixTag
{
};

/// Indicate that we want to operate on the system matrix of a linear system
inline boost::proto::result_of::make_expr< boost::proto::tag::function, SystemMatrixTag, StoredReference<Solver::CEigenLSS> >::type const
system_matrix(Solver::CEigenLSS& lss)
{
  return boost::proto::make_expr<boost::proto::tag::function>( SystemMatrixTag(), store(lss));
}
  
/// Tag for RHS
struct SystemRHSTag
{
};
  
/// Indicate that we want to operate on the RHS of a linear system
inline boost::proto::result_of::make_expr< boost::proto::tag::function, SystemRHSTag, StoredReference<Solver::CEigenLSS> >::type const
system_rhs(Solver::CEigenLSS& lss)
{
  return boost::proto::make_expr<boost::proto::tag::function>( SystemRHSTag(), store(lss) );
}

/// Grammar matching the LHS of an assignment op
template<typename TagT>
struct BlockLhsGrammar :
  boost::proto::function< boost::proto::terminal<TagT>, boost::proto::terminal< StoredReference<Solver::CEigenLSS> > >
{
};

/// Allowed block assignment operations
template<typename SystemTagT>
struct MatrixAssignOpsCases
{
  template<typename Tag, int Dummy = 0> struct case_ : boost::proto::not_<boost::proto::_> {};
  
  template<int Dummy> struct case_<boost::proto::tag::assign, Dummy> : boost::proto::assign<BlockLhsGrammar<SystemTagT> , boost::proto::_ > {};
  template<int Dummy> struct case_<boost::proto::tag::plus_assign, Dummy> : boost::proto::plus_assign<BlockLhsGrammar<SystemTagT> , boost::proto::_ > {};
  template<int Dummy> struct case_<boost::proto::tag::minus_assign, Dummy> : boost::proto::minus_assign<BlockLhsGrammar<SystemTagT> , boost::proto::_ > {};
};

/// Translate tag to operator
inline void do_assign_op(boost::proto::tag::assign, Real& lhs, const Real rhs)
{
  lhs = rhs;
}

/// Translate tag to operator
inline void do_assign_op(boost::proto::tag::plus_assign, Real& lhs, const Real rhs)
{
  lhs += rhs;
}

/// Translate tag to operator
inline void do_assign_op(boost::proto::tag::minus_assign, Real& lhs, const Real rhs)
{
  lhs -= rhs;
}

/// Helper struct for assignment to a matrix or RHS
template<typename SystemTagT, typename OpTagT>
struct BlockAssignmentOp;

template<typename OpTagT>
struct BlockAssignmentOp<SystemMatrixTag, OpTagT>
{
  template<typename RhsT, typename DataT>
  void operator()(Solver::CEigenLSS& lss, const RhsT& rhs, const DataT& data) const
  {
    // TODO: We take some shortcuts here that assume the same shape function for every variable. Storage order for the system is i.e. uvp, uvp, ...
    static const Uint mat_size = DataT::EMatrixSizeT::value;
    static const Uint nb_dofs = mat_size / DataT::SupportT::SF::nb_nodes;
    const Mesh::CTable<Uint>::ConstRow connectivity = data.support().element_connectivity();
    for(Uint row = 0; row != mat_size; ++row)
    {
      const Uint i_gid = connectivity[row % DataT::SupportT::SF::nb_nodes]*nb_dofs + row / DataT::SupportT::SF::nb_nodes;
      for(Uint col = 0; col != mat_size; ++col)
      {
        do_assign_op(OpTagT(), lss.at(i_gid, connectivity[col % DataT::SupportT::SF::nb_nodes]*nb_dofs + col / DataT::SupportT::SF::nb_nodes), rhs(row, col));
      }
    }
  }
};

template<typename OpTagT>
struct BlockAssignmentOp<SystemRHSTag, OpTagT>
{
  template<typename RhsT, typename DataT>
  void operator()(Solver::CEigenLSS& lss, const RhsT& rhs, const DataT& data) const
  {
    // TODO: We take some shortcuts here that assume the same shape function for every variable. Storage order for the system is i.e. uvp, uvp, ...
    static const Uint mat_size = DataT::EMatrixSizeT::value;
    static const Uint nb_dofs = mat_size / DataT::SupportT::SF::nb_nodes;
    const Mesh::CTable<Uint>::ConstRow connectivity = data.support().element_connectivity();
    for(Uint i = 0; i != mat_size; ++i)
    {
      do_assign_op(OpTagT(), lss.rhs()[connectivity[i % DataT::SupportT::SF::nb_nodes]*nb_dofs + i / DataT::SupportT::SF::nb_nodes], rhs[i]);
    }
  }
};

/// Primitive transform to handle assignment to an LSS matrix
struct BlockAccumulator :
  boost::proto::transform< BlockAccumulator >
{
  template<typename ExprT, typename State, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, State, DataT>
  {
    /// Contrary to general C++ assignment, assigning to a LSS doesn't return anything
    typedef void result_type;

    /// Tag type represents the actual operation type
    typedef typename boost::proto::tag_of<ExprT>::type OpT;
    
    /// Tag indicating if we are modifying the RHS or the system matrix (stored in child 0 of the LHS of the assignment op)
    typedef typename boost::remove_const
    <
      typename boost::remove_reference
      <
        typename boost::proto::result_of::value
        <
          typename boost::proto::result_of::child_c< typename boost::proto::result_of::left<ExprT>::type, 0 >::type
        >::type
      >::type
    >::type SystemTagT;

    result_type operator ()(
                typename impl::expr_param expr // The assignment expression
              , typename impl::state_param state // should be the element matrix, i.e. RHS already evaluated
              , typename impl::data_param data // data associated with element loop
    ) const
    {
      Solver::CEigenLSS& lss = boost::proto::value( boost::proto::child_c<1>( boost::proto::left(expr) ) ).get();
      BlockAssignmentOp<SystemTagT, OpT>()(lss, state, data);
    }
  };
};

/// Grammar matching block accumulation expressions
template<typename GrammarT>
struct BlockAccumulation : 
  boost::proto::when
  <
    boost::proto::or_< boost::proto::switch_< MatrixAssignOpsCases<SystemMatrixTag> >, boost::proto::switch_< MatrixAssignOpsCases<SystemRHSTag> > >,
    BlockAccumulator( boost::proto::_expr, GrammarT(boost::proto::_right) )
  >
{
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_BlockAccumulator_hpp
