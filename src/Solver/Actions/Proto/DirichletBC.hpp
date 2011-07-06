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

#include "LSSProxy.hpp"
#include "Terminals.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {
 
/// Tag for a Dirichlet BC
struct DirichletBCTag
{
};

/// Used to create placeholders for a Dirichlet condition
typedef LSSComponentTerm<DirichletBCTag> DirichletBC;

/// Helper function for assignment
inline void assign_dirichlet(CEigenLSS& lss, const Real new_value, const Real old_value, const Uint node_idx, const Uint offset, const Uint nb_dofs)
{
  // Index in the global system
  const Uint sys_idx = node_idx*nb_dofs + offset;
  lss.set_dirichlet_bc(sys_idx, new_value - old_value);
}

/// Overload for vector types
template<typename NewT, typename OldT>
inline void assign_dirichlet(CEigenLSS& lss, const NewT& new_value, const OldT& old_value, const Uint node_idx, const Uint offset, const Uint nb_dofs)
{
  // Index in the global system
  const Uint sys_idx = node_idx*nb_dofs + offset;
  for(Uint i = 0; i != OldT::RowsAtCompileTime; ++i)
    lss.set_dirichlet_bc(sys_idx+i, new_value[i] - old_value[i]);
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
      Solver::CEigenLSS& lss = boost::proto::value( boost::proto::child_c<0>(expr) ).lss_proxy().lss();
      assign_dirichlet(
        lss,
        state,
        data.var_data(boost::proto::value(boost::proto::child_c<1>(expr))).value(), // old value
        data.node_idx,
        data.var_data(boost::proto::value(boost::proto::child_c<1>(expr))).offset,
        data.nb_dofs()                  
      );
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
        boost::proto::terminal< LSSComponent<DirichletBCTag> >,
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
