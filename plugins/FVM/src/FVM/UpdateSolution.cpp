// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CMesh.hpp"
#include "FVM/UpdateSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < UpdateSolution, CAction, LibFVM > UpdateSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
UpdateSolution::UpdateSolution ( const std::string& name ) : 
  CAction(name)
{
  mark_basic();

  // options

  m_properties.add_option(OptionComponent<CField2>::create("Solution","Solution to update", &m_solution))
    ->add_tag("solution");

  m_properties.add_option(OptionComponent<CField2>::create("UpdateCoeff","Update coefficient", &m_update_coeff))
    ->add_tag("update_coeff");

  m_properties.add_option(OptionComponent<CField2>::create("Residual","Residual", &m_residual))
    ->add_tag("residual");
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSolution::execute()
{
  if (m_solution.expired())     throw SetupError(FromHere(), "Solution field was not set");
  if (m_residual.expired())     throw SetupError(FromHere(), "Residual field was not set");
  if (m_update_coeff.expired()) throw SetupError(FromHere(), "UpdateCoeff Field was not set");
  
  CTable<Real>& solution = m_solution.lock()->data();
  CTable<Real>& residual = m_residual.lock()->data();
  CTable<Real>& update_coeff = m_update_coeff.lock()->data();
  
  for (Uint i=0; i<solution.size(); ++i)
  {
    for (Uint j=0; j<solution.row_size(); ++j)
    {
      solution[i][j] += update_coeff[i][0] * residual[i][j];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

