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

#include "Math/MathChecks.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "RDM/Core/UpdateSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Math::MathChecks;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < UpdateSolution, CAction, LibCore > UpdateSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
UpdateSolution::UpdateSolution ( const std::string& name ) :
  RDM::Action(name)
{
  mark_basic();

  // options

  m_properties.add_option(OptionComponent<CField>::create("Solution","Solution field", &m_solution))
    ->add_tag("solution");

  m_properties.add_option(OptionComponent<CField>::create("WaveSpeed","Wave speed field", &m_wave_speed))
    ->add_tag("wave_speed");

  m_properties.add_option(OptionComponent<CField>::create("Residual","Residual field", &m_residual))
    ->add_tag("residual");
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

  const Real CFL = parent()->property("CFL").value<Real>();

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

