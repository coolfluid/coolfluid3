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
      //std::cout << "setting dirichlet bc for var: " << boost::proto::value(boost::proto::child_c<2>(expr)).variable_value.var_name << std::endl;
      Solver::CEigenLSS& lss = boost::proto::value( boost::proto::child_c<1>(expr) ).get();
      assign_dirichlet(
        lss,
        state,
        data.var_data(boost::proto::value(boost::proto::child_c<2>(expr))).value(), // old value
        data.node_idx,
        data.var_data(boost::proto::value(boost::proto::child_c<2>(expr))).offset,
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
