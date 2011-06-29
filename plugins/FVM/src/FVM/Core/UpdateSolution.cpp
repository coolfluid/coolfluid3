// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "FVM/Core/UpdateSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < UpdateSolution, CAction, LibCore > UpdateSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////

UpdateSolution::UpdateSolution ( const std::string& name ) :
  CAction(name)
{
  mark_basic();

  // options

  m_options.add_option(OptionComponent<CField>::create("solution","Solution","Solution to update", &m_solution));

  m_options.add_option(OptionComponent<CField>::create("update_coeff","Update Coeffient","Update coefficient", &m_update_coeff));

  m_options.add_option(OptionComponent<CField>::create("residual","Residual","Residual", &m_residual));
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

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

