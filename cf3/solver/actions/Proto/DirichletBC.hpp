// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_DirichletBC_hpp
#define cf3_solver_actions_Proto_DirichletBC_hpp

#include <boost/proto/core.hpp>

#include "math/MatrixTypes.hpp"

#include "math/LSS/System.hpp"

#include "ComponentWrapper.hpp"
#include "Terminals.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Tag for a Dirichlet BC
struct DirichletBCTag
{
};

/// Used to create placeholders for a Dirichlet condition
typedef ComponentWrapper<math::LSS::System, DirichletBCTag> DirichletBC;

/// Helper function for assignment
inline void assign_dirichlet(math::LSS::System& lss, const Real new_value, const Real old_value, const Uint node_idx, const Uint offset)
{
  lss.dirichlet(node_idx, offset, new_value - old_value);
}

/// Overload for vector types
template<typename NewT, typename OldT>
inline void assign_dirichlet(math::LSS::System& lss, const NewT& new_value, const OldT& old_value, const Uint node_idx, const Uint offset)
{
  for(Uint i = 0; i != OldT::RowsAtCompileTime; ++i)
    lss.dirichlet(node_idx, offset+i, new_value[i] - old_value[i]);
}

struct DirichletBCSetter :
  boost::proto::transform< DirichletBCSetter >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;

    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      math::LSS::System& lss = boost::proto::value( boost::proto::child_c<0>(expr) ).component();
      assign_dirichlet(
        lss,
        state,
        data.var_data(boost::proto::value(boost::proto::child_c<1>(expr))).value(), // old value
        data.node_idx,
        data.var_data(boost::proto::value(boost::proto::child_c<1>(expr))).offset
      );
    }
  };
};

/// Matches the proper formulation of Dirichlet BC
template<typename GrammarT>
struct DirichletBCGrammar :
  boost::proto::when
  <
    boost::proto::assign
    <
      boost::proto::function
      <
        boost::proto::terminal< ComponentWrapperImpl<math::LSS::System, DirichletBCTag> >,
        FieldTypes
      >,
      GrammarT
    >,
    DirichletBCSetter(boost::proto::_left, GrammarT(boost::proto::_right))
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_DirichletBC_hpp
