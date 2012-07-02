// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
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

  options().add( "cfl", 1.0 )
      .pretty_name("CFL")
      .description("Courant-Fredrichs-Levy stability number");

}

////////////////////////////////////////////////////////////////////////////////

void FwdEuler::execute()
{
  RDSolver& mysolver = *solver().handle< RDSolver >();

  if (is_null(m_solution))
    m_solution = follow_link(mysolver.fields().get_child( RDM::Tags::solution() ))->handle<Field>();
  if (is_null(m_wave_speed))
    m_wave_speed = follow_link(mysolver.fields().get_child( RDM::Tags::wave_speed() ))->handle<Field>();
  if (is_null(m_residual))
    m_residual = follow_link(mysolver.fields().get_child( RDM::Tags::residual() ))->handle<Field>();

  Field& solution     = *m_solution;
  Field& wave_speed   = *m_wave_speed;
  Field& residual     = *m_residual;

  CFinfo << "PPPPPPPPPPPPPP3: " << solution.uri().path() << CFendl;
  CFinfo << "PPPPPPPPPPPPPP4: " << residual.uri().path() << CFendl;
  CFinfo << "PPPPPPPPPPPPPP5: " << wave_speed.uri().path() << CFendl;

  const Real CFL = options().value<Real>("cfl");

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

