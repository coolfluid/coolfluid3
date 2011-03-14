// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_NeumannBC_hpp
#define CF_Solver_Actions_Proto_NeumannBC_hpp

#include <boost/proto/core.hpp>

#include "Math/MatrixTypes.hpp"
#include "Solver/CEigenLSS.hpp"

#include "Transforms.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {
 
/// Tag for a Neumann BC
struct NeumannBC
{
};

/// Placeholder to define dirichlet boundary conditions
template<Uint I, typename T>
inline typename boost::proto::result_of::make_expr< boost::proto::tag::function, NeumannBC, StoredReference<Solver::CEigenLSS>, MeshTerm<I, T> const & >::type const
neumann(Solver::CEigenLSS& lss, MeshTerm<I, T> const & var)
{
  return boost::proto::make_expr<boost::proto::tag::function>( NeumannBC(), store(lss), boost::ref(var) );
}
  
struct NeumannBCSetter :
  boost::proto::transform< NeumannBCSetter >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;
    
    result_type operator ()(
                typename impl::expr_param expr // Of the form neumann(lss, variable)
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      Solver::CEigenLSS& lss = boost::proto::value( boost::proto::child_c<1>(expr) ).get();
      const std::vector<Uint>& offsets = data.variable_offsets();
      const Uint sys_idx = data.node_idx*offsets.back() + offsets[boost::proto::value( boost::proto::child_c<2>(expr) )];
      lss.rhs()[sys_idx] = state;
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
        boost::proto::terminal<NeumannBC>,
        boost::proto::terminal< StoredReference<Solver::CEigenLSS> >,
        FieldTypes
      >,
      GrammarT
    >,
    NeumannBCSetter(boost::proto::_left, GrammarT(boost::proto::_right))
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_NeumannBC_hpp
