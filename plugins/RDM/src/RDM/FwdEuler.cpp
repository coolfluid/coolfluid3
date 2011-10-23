// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/Foreach.hpp"

#include "math/Checks.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/FwdEuler.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::math::Checks;

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < FwdEuler, common::Action, LibRDM > FwdEuler_Builder;

///////////////////////////////////////////////////////////////////////////////////////

FwdEuler::FwdEuler ( const std::string& name ) :
  cf3::solver::Action(name)
{
  mark_basic();

  m_options.add_option< OptionT<Real> >( "cfl", 1.0 )
      ->pretty_name("CFL")
      ->description("Courant-Fredrichs-Levy stability number");

}

////////////////////////////////////////////////////////////////////////////////

void FwdEuler::execute()
{
  RDSolver& mysolver = solver().as_type< RDSolver >();

  if (m_solution.expired())
    m_solution = mysolver.fields().get_child( RDM::Tags::solution() ).follow()->as_ptr_checked<Field>();
  if (m_wave_speed.expired())
    m_wave_speed = mysolver.fields().get_child( RDM::Tags::wave_speed() ).follow()->as_ptr_checked<Field>();
  if (m_residual.expired())
    m_residual = mysolver.fields().get_child( RDM::Tags::residual() ).follow()->as_ptr_checked<Field>();

  Field& solution     = *m_solution.lock();
  Field& wave_speed   = *m_wave_speed.lock();
  Field& residual     = *m_residual.lock();

  const Real CFL = options().option("cfl").value<Real>();

  const Uint nbdofs = solution.size();
  const Uint nbvars = solution.row_size();
  for ( Uint i=0; i< nbdofs; ++i )
  {
    if ( is_zero(wave_speed[i][0]) )
    {
      for ( Uint j=0; j< nbvars; ++j )
        if( is_not_zero(residual[i][j]) )
          CFwarn << "residual not null but wave_speed null at node [" << i << "] variable [" << j << "]" << CFendl;
      continue;
    }

    const Real update = CFL / wave_speed[i][0] ;
    for ( Uint j=0; j< nbvars; ++j )
      solution[i][j] += - update * residual[i][j];
  }
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

///////////////////////////////////////////////////////////////////////////////

