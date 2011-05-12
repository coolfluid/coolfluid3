// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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
#include "Solver/Actions/Proto/Terminals.hpp"

/// @file 
/// Transforms used in element-wise expression evaluation

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Primitive transform to extract the type of an SFOp
struct ExtractOp : boost::proto::transform< ExtractOp >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef typename boost::remove_reference<ExprT>::type::type result_type;
    
    result_type operator()(typename impl::expr_param, typename impl::state_param, typename impl::data_param)
    {
      return result_type();
    }
  };
};

/// Shape-function related operations
struct SFOps :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::function< boost::proto::terminal< SFOp<boost::proto::_> >, boost::proto::vararg<boost::proto::_> >,
      boost::proto::lazy< boost::proto::call<ExtractOp(boost::proto::_value(boost::proto::_child0))> >
    >,
    boost::proto::when
    <
      boost::proto::terminal< SFOp<boost::proto::_> >,
      boost::proto::lazy< boost::proto::call<ExtractOp(boost::proto::_value)> >
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
    SFOps,
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
