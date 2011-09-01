// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementTransforms_hpp
#define CF_Solver_Actions_Proto_ElementTransforms_hpp

#include <boost/mpl/assert.hpp>

#include <boost/proto/transform/lazy.hpp>

#include "Solver/Actions/Proto/EigenTransforms.hpp"
#include "Solver/Actions/Proto/ElementOperations.hpp"
#include "Solver/Actions/Proto/ElementData.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

/// @file 
/// Transforms used in element-wise expression evaluation

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Runs a shape function operation that is used as a terminal
template<typename GrammarT>
struct RunTerminalOp : boost::proto::transform< RunTerminalOp<GrammarT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<typename boost::proto::result_of::value<ExprT>::type>::type ValueT;
    typedef typename boost::result_of<ValueT(GrammarT)>::type OpT;
    typedef typename boost::result_of<OpT(typename impl::expr_param, typename impl::state_param, typename impl::data_param)>::type result_type;
    
    result_type operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data)
    {
      return OpT()(expr, state, data);
    }
  };
};
  
/// Runs a shape function operation that is used as a function call
template<typename GrammarT>
struct RunFunctionOp : boost::proto::transform< RunFunctionOp<GrammarT> >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::proto::result_of::child_c<ExprT, 0>::type Child0;
    typedef typename boost::remove_reference<typename boost::proto::result_of::value<Child0>::type>::type ValueT;
    typedef typename boost::result_of<ValueT(GrammarT)>::type OpT;
    typedef typename boost::result_of<OpT(typename impl::expr_param, typename impl::state_param, typename impl::data_param)>::type result_type;
    
    result_type operator()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data)
    {
      return OpT()(expr, state, data);
    }
  };
};

/// Shape-function related operations
template<typename GrammarT>
struct SFOps :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< SFOp<boost::proto::_> >, boost::proto::vararg<boost::proto::_> >,
      RunFunctionOp<GrammarT>
    >,
    boost::proto::when
    <
      boost::proto::terminal< SFOp<boost::proto::_> >,
      RunTerminalOp<GrammarT>
    >
  >
{
};

/// Filter to extract the value from an element-based field
struct ElementValue : boost::proto::transform<ElementValue>
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    
    typedef typename VarDataType<ExprT, DataT>::type VarDataT;
    // TODO: Result type deduction (needed only for element-based vector fields)
    typedef Real& result_type;
    
    result_type operator()(typename impl::expr_param var, typename impl::state_param, typename impl::data_param data)
    {
      return dispatch(typename VarDataT::SF(), data.var_data(var), var);
    }
    
    /// static dispatch in case of node-based field, giving an error
    template<typename SF>
    result_type dispatch(const SF&, VarDataT&, typename impl::expr_param var)
    {
      throw Common::SetupError(FromHere(), "Variable " + var.variable_value.name() + " is used like an element-based variable, but it is stored in a node-based field");
    }
    
    /// static dispatch in case of element-based field
    result_type dispatch(const ElementBased&, VarDataT& var, typename impl::expr_param)
    {
      return var.value();
    }
  };
};

struct NodalValuesTag
{
};

static boost::proto::terminal<NodalValuesTag>::type const nodal_values = {};

/// Get nodal values
struct NodalValues :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::function<boost::proto::terminal<NodalValuesTag>, FieldTypes>,
      VarValue(boost::proto::_value(boost::proto::_child1))
    >,
    boost::proto::when
    <
      FieldTypes,
      ElementValue(boost::proto::_value)
    >
  >
{
};

/// Interpolate a field at the current gauss point
struct FieldInterpolation :
  boost::proto::when
  <
    FieldTypes,
    InterpolationOp(boost::proto::_value)
  >
{
};

/// Expressions with implicitely known mapped coordinates, valid in i.e. integration, where the integrator
/// is responsible for setting the mapped coordinates of the points to evaluate
struct ElementMathImplicit :
  boost::proto::or_
  <
    SFOps< boost::proto::terminal<boost::proto::_> >,
    FieldInterpolation,
    MathTerminals,
    EigenMath<ElementMathImplicit, Integers>
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementTransforms_hpp
