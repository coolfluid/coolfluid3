// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_DirichletBC_hpp
#define CF_Solver_Actions_Proto_DirichletBC_hpp

#include <boost/proto/proto.hpp>

#include "Math/MatrixTypes.hpp"

#include "Solver/CEigenLSS.hpp"

#include "Transforms.hpp"

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
template<typename MatrixT, typename RhsT>
void assign_dirichlet(MatrixT& matrix, RhsT& rhs, const Real value, const std::vector<Uint>& offsets, const Uint node_idx, const Uint var_idx, const Real old_value)
{
  const int size = matrix.rows();
  cf_assert(matrix.cols() == size);
  cf_assert(rhs.rows() == size);
  
  // Index in the global system
  const Uint sys_idx = node_idx*offsets.back() + offsets[var_idx];
  
  matrix.row(sys_idx).setZero();
  matrix(sys_idx, sys_idx) = 1.;
  rhs[sys_idx] = value - old_value;
}
  
struct DirichletBCSetter :
  boost::proto::transform< DirichletBCSetter >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;
    
    /// Index for the variable
    typedef typename boost::result_of<VarNumber( typename boost::proto::result_of::child_c<ExprT, 2>::type )>::type I;
    
    result_type operator ()(
                typename impl::expr_param expr
              , typename impl::state_param state
              , typename impl::data_param data
    ) const
    {
      Solver::CEigenLSS& lss = boost::proto::value( boost::proto::child_c<1>(expr) ).get();
      assign_dirichlet( lss.matrix(), lss.rhs(), state, data.variable_offsets(), data.node_idx,
                        I::value,
                        data.template var_data<I>().value() );
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
        boost::proto::terminal< Var< boost::proto::_, Field<boost::proto::_> > >
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
