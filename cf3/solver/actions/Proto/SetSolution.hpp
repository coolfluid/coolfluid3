// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_SetSolution_hpp
#define cf3_solver_actions_Proto_SetSolution_hpp

#include <boost/proto/core.hpp>

#include "math/MatrixTypes.hpp"
#include "math/LSS/System.hpp"
#include "math/LSS/Vector.hpp"

#include "BlockAccumulator.hpp"
#include "Transforms.hpp"
#include "SolutionVector.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

struct SetSolutionSetter :
  boost::proto::transform< SetSolutionSetter >
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
      const int node_idx = boost::proto::value( boost::proto::child_c<0>(expr) ).node_to_lss(data.node_idx);
      if(node_idx < 0)
        return;
      typedef boost::mpl::int_< VarDataType<typename VarChild<ExprT, 1>::type, DataT>::type::dimension > DimT;
      set_value(DimT(), state, node_idx, data.var_data(boost::proto::value(boost::proto::child_c<1>(expr))).offset, boost::proto::value( boost::proto::child_c<0>(expr) ).solution());
    }

    template<typename DimT, typename ValueT>
    void set_value(DimT, const ValueT& val, const Uint node_idx, const Uint var_offset, math::LSS::Vector& vec) const
    {
      for(Uint i = 0; i != DimT::value; ++i)
      {
        const Uint eq_idx = var_offset + i;
        vec.set_value(node_idx, eq_idx, val[i]);
      }
    }

    void set_value(boost::mpl::int_<1>, const Real val, const Uint node_idx, const Uint eq_idx, math::LSS::Vector& vec) const
    {
      vec.set_value(node_idx, eq_idx, val);
    }

  };
};

/// Matches the proper formulation of Neumann BC
template<typename GrammarT>
struct SetSolutionGrammar :
  boost::proto::when
  <
    boost::proto::assign
    <
      boost::proto::function
      <
        boost::proto::terminal< LSSWrapperImpl<SolutionVectorTag> >,
        FieldTypes
      >,
      GrammarT
    >,
    SetSolutionSetter(boost::proto::_left, GrammarT(boost::proto::_right))
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_SetSolution_hpp
