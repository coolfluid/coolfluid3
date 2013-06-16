// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_Functions_hpp
#define cf3_solver_actions_Proto_Functions_hpp

#include <boost/proto/core.hpp>

#include "common/CF.hpp"
#include "math/VectorialFunction.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Base class of all functions that can be evaluated using "default" C++ semantics
struct FunctionBase
{
};

/// Pow function based on Proto docs example
template<Uint Exp>
struct pow_fun : FunctionBase
{
  typedef Real result_type;
  Real operator()(Real d) const
  {
    for(Uint i = 0; i != (Exp-1); ++i)
      d *= d;
    return d;
  }
};

template<Uint Exp, typename Arg>
typename boost::proto::result_of::make_expr<
    boost::proto::tag::function  // Tag type
  , pow_fun<Exp>          // First child (by value)
  , Arg const &           // Second child (by reference)
>::type const
pow(Arg const &arg)
{
  return boost::proto::make_expr<boost::proto::tag::function>(
      pow_fun<Exp>()    // First child (by value)
    , boost::ref(arg)   // Second child (by reference)
  );
}


/// Wrap the vectorial function, adding extra data that may be filled before expression evaluation
struct ProtoEvaluatedFunction : math::VectorialFunction
{
  mutable std::vector<Real> predefined_values;
};


struct ScalarFunction : ProtoEvaluatedFunction
{
};

struct VectorFunction : ScalarFunction
{
};

template<typename ResultT, typename CoordsT>
void evaluate_function(const ProtoEvaluatedFunction& func, const CoordsT& coords, ResultT& result)
{
  cf3_assert(func.predefined_values.size() >= CoordsT::RowsAtCompileTime);
  for(int i = 0; i != CoordsT::RowsAtCompileTime; ++i)
  {
    func.predefined_values[i] = coords[i];
  }

  func.evaluate(func.predefined_values, result);
}

/// Primitive transform to evaluate a function with the function parser
struct ParsedVectorFunctionTransform :
  boost::proto::transform< ParsedVectorFunctionTransform >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<DataT>::type::CoordsT result_type;

    result_type operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      result_type result;
      evaluate_function(boost::proto::value(expr), data.coordinates(), result);
      return result;
    }
  };
};

struct ParsedScalarFunctionTransform :
boost::proto::transform< ParsedScalarFunctionTransform >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef Real result_type;

    Real operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      std::vector<Real> result(1);
      evaluate_function(boost::proto::value(expr), data.coordinates(), result);
      return result.back();
    }
  };
};

struct ParsedFunctionGrammar :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::terminal<VectorFunction>,
      ParsedVectorFunctionTransform
    >,
    boost::proto::when
    <
      boost::proto::terminal<ScalarFunction>,
      ParsedScalarFunctionTransform
    >
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_Functions_hpp
