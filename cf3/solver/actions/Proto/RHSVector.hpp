// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_RHSVector_hpp
#define cf3_solver_actions_Proto_RHSVector_hpp

#include <boost/proto/core.hpp>

#include "math/MatrixTypes.hpp"
#include "math/LSS/System.hpp"
#include "math/LSS/Vector.hpp"

#include "BlockAccumulator.hpp"
#include "LSSWrapper.hpp"
#include "Transforms.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

struct GetRHSVector :
  boost::proto::transform< GetRHSVector >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {

    typedef typename impl::data::template DataType<typename impl::state>::type VarDataT;
    typedef typename VarDataT::ValueT result_type;

    // Used in case result is a scalar
    Real& result_at(Real& result, const Uint) const
    {
      return result;
    }

    // Used if the result is a vector
    template<typename T>
    Real& result_at(T& result, const Uint i) const
    {
      return result[i];
    }

    result_type operator ()(
                typename impl::expr_param expr // Of the form Neumann(lss, variable)
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      math::LSS::Vector& rhs = expr.rhs();
      const int node_idx = expr.node_to_lss(data.node_idx);
      if(node_idx < 0)
        throw common::SetupError(FromHere(), "RHS access error: attempt to use node that is not in the LSS");
      const Uint sys_idx = node_idx*data.var_data(state).nb_dofs + data.var_data(state).offset;
      result_type result;
      for(Uint i = 0; i != VarDataT::dimension; ++i)
        rhs.get_value(sys_idx+i, result_at(result, i));
      return result;
    }
  };
};

/// Matches placeholders for the solution vector
struct RHSVectorGrammar :
  boost::proto::when
  <
    boost::proto::function
    <
      boost::proto::terminal< LSSWrapperImpl<SystemRHSTag> >,
      FieldTypes
    >,
    GetRHSVector(boost::proto::_value(boost::proto::_child0), boost::proto::_value(boost::proto::_child1))
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_RHSVector_hpp
