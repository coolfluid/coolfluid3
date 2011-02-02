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
inline boost::proto::result_of::make_expr< boost::proto::tag::function, DirichletBC, StoredReference<Solver::CEigenLSS> >::type const
dirichlet(Solver::CEigenLSS& arg)
{
  return boost::proto::make_expr<boost::proto::tag::function>( DirichletBC(), store(arg) );
}

/// Helper function for assignment
template<typename MatrixT, typename RhsT>
void assign_dirichlet(MatrixT& matrix, RhsT& rhs, const Real value, const Uint node_idx)
{
  const int size = matrix.rows();
  cf_assert(matrix.cols() == size);
  cf_assert(rhs.rows() == size);
  
  matrix.row(node_idx).setZero();
  matrix(node_idx, node_idx) = 1.;
  rhs[node_idx] = value;
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
      Solver::CEigenLSS& lss = boost::proto::value(expr).get();
      assign_dirichlet(lss.matrix(), lss.rhs(), state, data.node_idx);
    }
  };
};

/// Matches the proper formulation of Dirichlet BC
struct DirichletBCGrammar :
  boost::proto::assign
  <
    boost::proto::function
    <
      boost::proto::terminal<DirichletBC>,
      boost::proto::terminal< StoredReference<Solver::CEigenLSS> >
    >,
    boost::proto::_
  >
{
};
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_DirichletBC_hpp
