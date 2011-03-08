// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CField2.hpp"
#include "Mesh/CMesh.hpp"

#include "RDM/UpdateSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < UpdateSolution, CAction, LibRDM > UpdateSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
UpdateSolution::UpdateSolution ( const std::string& name ) :
  RDM::Action(name),
  m_cfl(1.0)
{
  mark_basic();

  // options

  m_properties.add_option< OptionT<Real> > ("CFL", "Courant Number", m_cfl)
      ->mark_basic()
      ->link_to(&m_cfl)
      ->add_tag("cfl");

  m_properties.add_option(OptionComponent<CField2>::create("Solution","Solution field", &m_solution))
    ->add_tag("solution");

  m_properties.add_option(OptionComponent<CField2>::create("WaveSpeed","Wave speed field", &m_wave_speed))
    ->add_tag("wave_speed");

  m_properties.add_option(OptionComponent<CField2>::create("Residual","Residual field", &m_residual))
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

  const Uint nbdofs = solution.size();
  const Uint nbvars = solution.row_size();
  for ( Uint i=0; i< nbdofs; ++i )
    for ( Uint j=0; j< nbvars; ++j )
      solution[i][j] += - (  m_cfl / wave_speed[i][j] ) * residual[i][j];

}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

