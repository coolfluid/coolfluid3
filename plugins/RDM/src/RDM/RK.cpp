// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

#include "math/Checks.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"

#include "RK.hpp"


using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::math::Checks;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < RK, common::Action, LibRDM > RK_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

RK::RK ( const std::string& name ) :
  cf3::solver::Action(name)
{
  mark_basic();

  // options

  options().add(RDM::Tags::solution(), m_solution).link_to(&m_solution);
  options().add(RDM::Tags::dual_area(), m_dual_area).link_to(&m_dual_area);
  options().add(RDM::Tags::residual(), m_residual).link_to(&m_residual);

  options().add( "cfl", 1.0 )
      .pretty_name("CFL")
      .description("Courant-Fredrichs-Levy stability number");

  options().add( "rkorder", 1u )
      .pretty_name("RK Order")
      .description("Order of the Runge-Kutta step");

}

void RK::execute()
{
  RDSolver& mysolver = *solver().handle< RDSolver >();

  // get the current rk k step and order

  const Uint rkorder = mysolver.properties().value<Uint>("rkorder");
  const Uint step    = mysolver.iterative_solver().properties().value<Uint>("iteration");

  if (is_null(m_solution))
    m_solution = follow_link( mysolver.fields().get_child( RDM::Tags::solution() ) )->handle<Field>();
  if (is_null(m_residual))
    m_residual = follow_link( mysolver.fields().get_child( RDM::Tags::residual() ) )->handle<Field>();
  if (is_null(m_dual_area))
    m_dual_area = follow_link( mysolver.fields().get_child( RDM::Tags::dual_area() ) )->handle<Field>();

  // get the correct solution to update depending on which rk k step we are

  Handle< Field > csolution_k;
  if ( step == rkorder )
    csolution_k = m_solution;
  else
  {
    csolution_k = follow_link(mysolver.fields().get_child( std::string(RDM::Tags::solution()) + "-" + to_str(step) + "dt" ))->handle<Field>();
  }

  cf3_assert( is_not_null(csolution_k) );

  Field& solution_k   = *csolution_k;
  Field& dual_area    = *m_dual_area;
  Field& residual     = *m_residual;

  /// @todo should be used later to calculate automatically the \Delta t
     //const Real CFL = options().value<Real>("cfl");

  /// @todo maybe better to directly store dual_area inverse

  // implementation of the RungeKutta update step

  const Uint nbdofs = solution_k.size();
  const Uint nbvars = solution_k.row_size();
  for ( Uint i=0; i< nbdofs; ++i )
    for ( Uint j=0; j< nbvars; ++j )
      solution_k[i][j] += - residual[i][j] / dual_area[i][0];
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
