// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_Solver_Actions_Proto_BlockAccumulator_hpp
#define cf3_Solver_Actions_Proto_BlockAccumulator_hpp

#include <boost/mpl/assert.hpp>
#include <boost/proto/core.hpp>
#include <boost/proto/traits.hpp>


#include "Math/MatrixTypes.hpp"

#include "Math/LSS/System.hpp"
#include "Math/LSS/BlockAccumulator.hpp"
#include "Math/LSS/Matrix.hpp"

#include "ComponentWrapper.hpp"
#include "Terminals.hpp"

/// @file
/// System matrix block accumulation.

namespace cf3 {
namespace Solver {
namespace Actions {
namespace Proto {

/// Tag for system matrix
struct SystemMatrixTag
{
};

/// Represents a system matrix
typedef ComponentWrapper<Math::LSS::System, SystemMatrixTag> SystemMatrix;

/// Tag for RHS
struct SystemRHSTag
{
};

/// Represents an RHS
typedef ComponentWrapper<Math::LSS::System, SystemRHSTag> SystemRHS;

/// Grammar matching the LHS of an assignment op
template<typename TagT>
struct BlockLhsGrammar :
  boost::proto::terminal< ComponentWrapperImpl<Math::LSS::System, TagT> >
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
inline void do_assign_op_matrix(boost::proto::tag::assign, Math::LSS::Matrix& lss_matrix, const Math::LSS::BlockAccumulator& block_accumulator)
{
  lss_matrix.set_values(block_accumulator);
}

/// Translate tag to operator
inline void do_assign_op_matrix(boost::proto::tag::plus_assign, Math::LSS::Matrix& lss_matrix, const Math::LSS::BlockAccumulator& block_accumulator)
{
  lss_matrix.add_values(block_accumulator);
}

/// Translate tag to operator
inline void do_assign_op_rhs(boost::proto::tag::assign, Math::LSS::Vector& lss_rhs, const Math::LSS::BlockAccumulator& block_accumulator)
{
  lss_rhs.set_rhs_values(block_accumulator);
}

/// Translate tag to operator
inline void do_assign_op_rhs(boost::proto::tag::plus_assign, Math::LSS::Vector& lss_rhs, const Math::LSS::BlockAccumulator& block_accumulator)
{
  lss_rhs.add_rhs_values(block_accumulator);
}

/// Translate tag to operator
template<typename TagT, typename TargetT>
inline void do_assign_op(TagT, Real& lhs, TargetT& lss_matrix, const Math::LSS::BlockAccumulator& block_accumulator)
{
  BOOST_MPL_ASSERT_MSG(
    false
    , UNSUPPORTED_ASSIGNMENT_OPERATION
    , (TagT)
  );
}

/// Helper struct for assignment to a matrix or RHS
template<typename SystemTagT, typename OpTagT>
struct BlockAssignmentOp;

template<typename OpTagT>
struct BlockAssignmentOp<SystemMatrixTag, OpTagT>
{
  template<typename RhsT, typename DataT>
  void operator()(Math::LSS::System& lss, const RhsT& rhs, const DataT& data) const
  {
    // TODO: We take some shortcuts here that assume the same shape function for every variable. Storage order for the system is i.e. uvp, uvp, ...
    static const Uint mat_size = DataT::EMatrixSizeT::value;
    static const Uint nb_dofs = mat_size / DataT::SupportT::EtypeT::nb_nodes;
    Math::LSS::BlockAccumulator& block_accumulator = data.block_accumulator;

    for(Uint row = 0; row != mat_size; ++row)
    {
      // This converts u1,u2...pn to u1v1p1...
      const Uint block_row = (row % DataT::SupportT::EtypeT::nb_nodes)*nb_dofs + row / DataT::SupportT::EtypeT::nb_nodes;
      for(Uint col = 0; col != mat_size; ++col)
      {
        const Uint block_col = (col % DataT::SupportT::EtypeT::nb_nodes)*nb_dofs + col / DataT::SupportT::EtypeT::nb_nodes;
        block_accumulator.mat(block_row, block_col) = rhs(row, col);
      }
    }
    do_assign_op_matrix(OpTagT(), *lss.matrix(), block_accumulator);
  }
};

template<typename OpTagT>
struct BlockAssignmentOp<SystemRHSTag, OpTagT>
{
  template<typename RhsT, typename DataT>
  void operator()(Math::LSS::System& lss, const RhsT& rhs, const DataT& data) const
  {
    // TODO: We take some shortcuts here that assume the same shape function for every variable. Storage order for the system is i.e. uvp, uvp, ...
    static const Uint mat_size = DataT::EMatrixSizeT::value;
    static const Uint nb_dofs = mat_size / DataT::SupportT::EtypeT::nb_nodes;
    Math::LSS::BlockAccumulator& block_accumulator = data.block_accumulator;
    for(Uint i = 0; i != mat_size; ++i)
    {
      // This converts u1,u2...pn to u1v1p1...
      const Uint block_idx = (i % DataT::SupportT::EtypeT::nb_nodes)*nb_dofs + i / DataT::SupportT::EtypeT::nb_nodes;
      block_accumulator.rhs[block_idx] = rhs[i];
    }

    do_assign_op_rhs(OpTagT(), *lss.rhs(), block_accumulator);
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
    typedef typename boost::remove_reference
    <
      typename boost::proto::result_of::value
      <
        typename boost::proto::result_of::left<ExprT>::type
      >::type
    >::type::tag_type SystemTagT;

    result_type operator ()(
                typename impl::expr_param expr // The assignment expression
              , typename impl::state_param state // should be the element matrix, i.e. RHS already evaluated
              , typename impl::data_param data // data associated with element loop
    ) const
    {
      BlockAssignmentOp<SystemTagT, OpT>()(boost::proto::value( boost::proto::left(expr) ).component(), state, data);
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
} // namespace cf3

#endif // cf3_Solver_Actions_Proto_BlockAccumulator_hpp
