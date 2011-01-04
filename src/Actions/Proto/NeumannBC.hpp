// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_Proto_NeumannBC_hpp
#define CF_Actions_Proto_NeumannBC_hpp

#include <boost/proto/proto.hpp>

#include "Math/MatrixTypes.hpp"

#include "Transforms.hpp"

namespace CF {
namespace Actions {
namespace Proto {
 
/// Tag for a Neumann BC
struct NeumannBC
{
};

/// Placeholder to define dirichlet boundary conditions
static boost::proto::terminal<NeumannBC>::type const neumann = {{}};
  
struct NeumannBCSetter :
  boost::proto::transform< NeumannBCSetter >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;
    
    typedef typename boost::result_of
    <
      VarValue
      (
        ExprT,
        StateT,
        typename boost::result_of<NumberedData(ExprT, StateT, DataT)>::type
      )
    >::type LSST;
    
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      LSST var = VarValue()(expr, state, NumberedData()(expr, state, data));
      var.lss().rhs()[data.node_idx] = state;
    }
  };
};

/// Matches the proper formulation of Neumann BC
struct NeumannBCGrammar :
  boost::proto::assign
  <
    boost::proto::function
    <
      boost::proto::terminal<NeumannBC>,
      boost::proto::terminal< Var< boost::proto::_, LSS > >
    >,
    boost::proto::_
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_Proto_NeumannBC_hpp
