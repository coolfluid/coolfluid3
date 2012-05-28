// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_NeumannBC_hpp
#define cf3_solver_actions_Proto_NeumannBC_hpp

#include <boost/proto/core.hpp>

#include "math/MatrixTypes.hpp"
#include "math/LSS/System.hpp"
#include "math/LSS/Vector.hpp"

#include "Transforms.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Tag for a Neumann BC
struct NeumannBCTag
{
};

/// Used to create placeholders for a Neumann condition
typedef LSSWrapper<NeumannBCTag> NeumannBC;

struct NeumannBCSetter :
  boost::proto::transform< NeumannBCSetter >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;

    result_type operator ()(
                typename impl::expr_param expr // Of the form Neumann(lss, variable)
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      const Uint node_idx = boost::proto::value( boost::proto::child_c<0>(expr) ).node_to_lss(data.node_idx);
      const Uint sys_idx = node_idx*data.var_data(boost::proto::value(boost::proto::child_c<1>(expr))).nb_dofs + data.var_data(boost::proto::value(boost::proto::child_c<1>(expr))).offset;
      boost::proto::value( boost::proto::child_c<0>(expr) ).rhs().set_value(sys_idx, state);
    }
  };
};

/// Matches the proper formulation of Neumann BC
template<typename GrammarT>
struct NeumannBCGrammar :
  boost::proto::when
  <
    boost::proto::assign
    <
      boost::proto::function
      <
        boost::proto::terminal< LSSWrapperImpl<NeumannBCTag> >,
        FieldTypes
      >,
      GrammarT
    >,
    NeumannBCSetter(boost::proto::_left, GrammarT(boost::proto::_right))
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_NeumannBC_hpp
