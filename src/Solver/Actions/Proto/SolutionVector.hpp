// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_SolutionVector_hpp
#define CF_Solver_Actions_Proto_SolutionVector_hpp

#include <boost/proto/core.hpp>

#include "Math/MatrixTypes.hpp"
#include "Solver/CEigenLSS.hpp"

#include "ComponentWrapper.hpp"
#include "Transforms.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Tag for a Neumann BC
struct SolutionVectorTag
{
};

/// Used to create placeholders for a Neumann condition
typedef ComponentWrapper<CEigenLSS, SolutionVectorTag> SolutionVector;

struct GetSolutionVector :
  boost::proto::transform< GetSolutionVector >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {

    typedef typename impl::data::template DataType<typename impl::state>::type VarDataT;
    typedef typename VarDataT::ValueT result_type;

    template<typename T>
    void convert_result(Real& output, const T& input) const
    {
      output = input[0];
    }

    template<typename T1, typename T2>
    void convert_result(T1& output, const T2& input) const
    {
      output = input;
    }

    result_type operator ()(
                typename impl::expr_param expr // Of the form neumann(lss, variable)
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      Solver::CEigenLSS& lss = expr.component();
      const Uint sys_idx = data.node_idx*data.var_data(state).nb_dofs + data.var_data(state).offset;
      result_type result;
      convert_result(result, lss.solution().segment<VarDataT::dimension>(sys_idx));
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
      boost::proto::terminal< ComponentWrapperImpl<CEigenLSS, SolutionVectorTag> >,
      FieldTypes
    >,
    GetSolutionVector(boost::proto::_value(boost::proto::_child0), boost::proto::_value(boost::proto::_child1))
  >
{
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_SolutionVector_hpp
