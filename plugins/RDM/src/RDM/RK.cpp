// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"

#include "Math/Checks.hpp"

#include "Mesh/Field.hpp"
#include "Mesh/CMesh.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"

#include "RK.hpp"


using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Math::Checks;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < RK, CAction, LibRDM > RK_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

RK::RK ( const std::string& name ) :
  CF::Solver::Action(name)
{
  mark_basic();

  // options

  options().add_option(
        Common::OptionComponent<Mesh::Field>::create( RDM::Tags::solution(), &m_solution));
  options().add_option(
        Common::OptionComponent<Mesh::Field>::create( RDM::Tags::dual_area(), &m_dual_area));
  options().add_option(
        Common::OptionComponent<Mesh::Field>::create( RDM::Tags::residual(), &m_residual));

  options().add_option< OptionT<Real> >( "cfl", 1.0 )
      ->pretty_name("CFL")
      ->description("Courant-Fredrichs-Levy stability number");

  options().add_option< OptionT<Real> >( "rkorder", 1u )
      ->pretty_name("RK Order")
      ->description("Order of the Runge-Kutta step");

}

void RK::execute()
{
  RDSolver& mysolver = solver().as_type< RDSolver >();

  // get the current rk k step and order

  const Uint rkorder = mysolver.properties().value<Uint>("rkorder");
  const Uint step    = mysolver.iterative_solver().properties().value<Uint>("iteration");

  if (m_solution.expired())
    m_solution = mysolver.fields().get_child( RDM::Tags::solution() ).follow()->as_ptr_checked<Field>();
  if (m_residual.expired())
    m_residual = mysolver.fields().get_child( RDM::Tags::residual() ).follow()->as_ptr_checked<Field>();
  if (m_dual_area.expired())
    m_dual_area = mysolver.fields().get_child( RDM::Tags::dual_area() ).follow()->as_ptr_checked<Field>();

  // get the correct solution to update depending on which rk k step we are

  CField::Ptr csolution_k;
  if ( step == rkorder )
   csolution_k = m_solution.lock();
  else
    csolution_k = find_component_ptr_with_name<CField>( mesh(), RDM::Tags::solution() + to_str(step) );

  cf_assert( is_not_null(csolution_k) );

  Field& solution_k   = *csolution_k;
  Field& dual_area    = *m_dual_area.lock();
  Field& residual     = *m_residual.lock();

  /// @todo should be used later to calculate automatically the \Delta t
     //const Real CFL = options().option("cfl").value<Real>();

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
} // CF
