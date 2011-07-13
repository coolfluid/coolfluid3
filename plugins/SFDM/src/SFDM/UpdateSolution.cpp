// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "SFDM/UpdateSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < UpdateSolution, CAction, LibSFDM > UpdateSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////

UpdateSolution::UpdateSolution ( const std::string& name ) :
  CAction(name)
{
  mark_basic();

  // options

  m_options.add_option(OptionComponent<CField>::create("solution",,, &m_solution))
     ->set_description("Solution to update")
     ->set_pretty_name("Solution");

  m_options.add_option(OptionComponent<CField>::create("update_coeff", &m_update_coeff))
     ->set_description("Update coefficient")
     ->set_pretty_name("Update Coefficient");

  m_options.add_option(OptionComponent<CField>::create("residual", &m_residual))
     ->set_description("Residual")
     ->set_pretty_name("Residual");

  m_solution_view = create_static_component_ptr<CMultiStateFieldView>("solution_view");
  m_residual_view = create_static_component_ptr<CMultiStateFieldView>("residual_view");
  m_update_coeff_view = create_static_component_ptr<CScalarFieldView>("update_coeff_view");
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSolution::execute()
{
  if (m_solution.expired())     throw SetupError(FromHere(), "Solution field was not set");
  if (m_residual.expired())     throw SetupError(FromHere(), "Residual field was not set");
  if (m_update_coeff.expired()) throw SetupError(FromHere(), "UpdateCoeff Field was not set");

  CScalarFieldView& update_coeff_view = *m_update_coeff_view;
  CMultiStateFieldView& residual_view = *m_residual_view;
  CMultiStateFieldView& solution_view = *m_solution_view;

  /// @todo put this in attached triggers of the field configuration
  residual_view.set_field(m_residual.lock());
  solution_view.set_field(m_solution.lock());
  update_coeff_view.set_field(m_update_coeff.lock());

  boost_foreach(const CEntities& elements, m_solution.lock()->field_elements())
  {
    residual_view.set_elements(elements);
    solution_view.set_elements(elements);
    update_coeff_view.set_elements(elements);

    for (Uint e=0; e<elements.size(); ++e)
    {
      CMultiStateFieldView::View solution = solution_view[e];
      CMultiStateFieldView::View residual = residual_view[e];
      Real update_coeff = update_coeff_view[e];
      for (Uint i=0; i<solution.shape()[0]; ++i)
      {
        for (Uint j=0; j<solution.shape()[1]; ++j)
        {
          solution[i][j] += update_coeff * residual[i][j];
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

