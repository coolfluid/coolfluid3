// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_DirichletBC_hpp
#define CF_Solver_Actions_Proto_DirichletBC_hpp

#include <boost/proto/core.hpp>

#include "Math/MatrixTypes.hpp"

#include "Solver/CEigenLSS.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {
 
/// Tag for a Dirichlet BC
struct DirichletBC
{
};

/// Placeholder to define dirichlet boundary conditions
template<Uint I, typename T>
inline typename boost::proto::result_of::make_expr< boost::proto::tag::function, DirichletBC, StoredReference<Solver::CEigenLSS>, MeshTerm<I, T> const & >::type const
dirichlet(Solver::CEigenLSS& lss, MeshTerm<I, T> const & var)
{
  return boost::proto::make_expr<boost::proto::tag::function>( DirichletBC(), store(lss), boost::ref(var) );
}

/// Helper function for assignment
inline void assign_dirichlet(CEigenLSS& lss, const Real value, const std::vector<Uint>& offsets, const Uint node_idx, const Uint var_idx, const Real old_value)
{
  // Index in the global system
  const Uint sys_idx = node_idx*offsets.back() + offsets[var_idx];
  
  lss.set_dirichlet_bc(sys_idx, value - old_value);
}

/// Overload for vector types
template<typename NewT, typename OldT>
inline void assign_dirichlet(CEigenLSS& lss, const NewT& value, const std::vector<Uint>& offsets, const Uint node_idx, const Uint var_idx, const OldT& old_value)
{
  // Index in the global system
  const Uint sys_idx = node_idx*offsets.back() + offsets[var_idx];
  for(Uint i = 0; i != OldT::RowsAtCompileTime; ++i)
    lss.set_dirichlet_bc(sys_idx+i, value[i] - old_value[i]);
}
  
struct DirichletBCSetter :
  boost::proto::transform< DirichletBCSetter >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;
    
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      Solver::CEigenLSS& lss = boost::proto::value( boost::proto::child_c<1>(expr) ).get();
      assign_dirichlet( lss, state, data.variable_offsets(), data.node_idx,
                        boost::proto::value(boost::proto::child_c<2>(expr)),
                        data.var_data(boost::proto::value(boost::proto::child_c<2>(expr))).value() );
    }
  };
};

/// Matches the proper formulation of Dirichlet BC
template<typename GrammarT>
struct DirichletBCGrammar :
  boost::proto::when
  <
    boost::proto::assign
    <
      boost::proto::function
      <
        boost::proto::terminal<DirichletBC>,
        boost::proto::terminal< StoredReference<Solver::CEigenLSS> >,
        FieldTypes
      >,
      GrammarT
    >,
    DirichletBCSetter(boost::proto::_left, GrammarT(boost::proto::_right))
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_DirichletBC_hpp
