// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_SolutionVector_hpp
#define cf3_solver_actions_Proto_SolutionVector_hpp

#include <boost/proto/core.hpp>

#include "math/MatrixTypes.hpp"
#include "math/LSS/System.hpp"
#include "math/LSS/Vector.hpp"

#include "LSSWrapper.hpp"
#include "Transforms.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Tag for a Neumann BC
struct SolutionVectorTag
{
};

/// Used to create placeholders for a Neumann condition
typedef LSSWrapper<SolutionVectorTag> SolutionVector;

struct GetSolutionVector :
  boost::proto::transform< GetSolutionVector >
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
      math::LSS::Vector& sol = expr.solution();
      const Uint node_idx = expr.node_to_lss(data.node_idx);
      const Uint sys_idx = node_idx*data.var_data(state).nb_dofs + data.var_data(state).offset;
      result_type result;
      for(Uint i = 0; i != VarDataT::dimension; ++i)
        sol.get_value(sys_idx+i, result_at(result, i));
      return result;
    }
  };
};

/// Matches placeholders for the solution vector
struct SolutionVectorGrammar :
  boost::proto::when
  <
    boost::proto::function
    <
      boost::proto::terminal< LSSWrapperImpl<SolutionVectorTag> >,
      FieldTypes
    >,
    GetSolutionVector(boost::proto::_value(boost::proto::_child0), boost::proto::_value(boost::proto::_child1))
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_SolutionVector_hpp
