// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/Foreach.hpp"

#include "Math/Checks.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "RDM/Core/UpdateSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Math::Checks;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < UpdateSolution, CAction, Core::LibCore > UpdateSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////

UpdateSolution::UpdateSolution ( const std::string& name ) :
  Solver::Action(name)
{
  mark_basic();

  // options

  m_options.add_option(OptionComponent<CField>::create("solution", &m_solution))
      ->set_description("Solution field")
      ->set_pretty_name("Solution");

  m_options.add_option(OptionComponent<CField>::create("wave_speed", &m_wave_speed))
      ->set_description("Wave speed field")
      ->set_pretty_name("WaveSpeed");

  m_options.add_option(OptionComponent<CField>::create("residual", &m_residual))
      ->set_description("Residual field")
      ->set_pretty_name("Residual");

}

////////////////////////////////////////////////////////////////////////////////

void UpdateSolution::execute()
{
  if (m_solution.expired())   throw SetupError(FromHere(), "Solution field was not set");
  if (m_wave_speed.expired()) throw SetupError(FromHere(), "WaveSpeed Field was not set");
  if (m_residual.expired())   throw SetupError(FromHere(), "Residual field was not set");

  CTable<Real>& solution     = m_solution.lock()->data();
  CTable<Real>& wave_speed   = m_wave_speed.lock()->data();
  CTable<Real>& residual     = m_residual.lock()->data();

  const Real CFL = parent().option("cfl").value<Real>();

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
} // CF

///////////////////////////////////////////////////////////////////////////////

