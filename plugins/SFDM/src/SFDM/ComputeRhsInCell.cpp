// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"

#include "SFDM/ComputeRhsInCell.hpp"
#include "SFDM/Reconstruct.hpp"
#include "SFDM/Flux.hpp"
#include "SFDM/ShapeFunction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ComputeRhsInCell, Solver::Actions::CLoopOperation, LibSFDM > ComputeRhsInCell_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
ComputeRhsInCell::ComputeRhsInCell ( const std::string& name ) :
  Solver::Actions::CLoopOperation(name)
{
  // options
  m_properties.add_option(OptionURI::create("solution","Solution","Solution to calculate RHS for", URI("cpath:"),URI::Scheme::CPATH))
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeRhsInCell::config_solution,   this ) )
    ->add_tag("solution");
  
  m_properties.add_option(OptionURI::create("residual","Residual","Residual to be calculated", URI("cpath:"),URI::Scheme::CPATH))
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeRhsInCell::config_residual,   this ) )
    ->add_tag("residual");

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &ComputeRhsInCell::trigger_elements,   this ) );

  m_solution = create_static_component_ptr<CMultiStateFieldView>("solution_view");
  m_residual = create_static_component_ptr<CMultiStateFieldView>("residual_view");

  m_reconstruct_solution = create_static_component_ptr<Reconstruct>("solution_reconstruction_helper");
  m_reconstruct_flux     = create_static_component_ptr<Reconstruct>("flux_reconstruction_helper");

  m_flux = create_static_component_ptr<Flux>("flux_helper");

}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::config_solution()
{
  URI uri;
  property("solution").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_solution->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::config_residual()
{
  URI uri;
  property("residual").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_residual->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::trigger_elements()
{
  m_can_start_loop = m_solution->set_elements(elements());
  m_can_start_loop = m_residual->set_elements(elements());

  if (m_can_start_loop)
  {
    std::vector<std::string> solution_from_to(2);
    solution_from_to[0] = elements().space("solution").shape_function().derived_type_name();
    solution_from_to[1] = elements().space("flux").shape_function().derived_type_name();
    m_reconstruct_solution->configure_property("from_to",solution_from_to);

    std::vector<std::string> flux_from_to(2);
    flux_from_to[0] = solution_from_to[1];
    flux_from_to[1] = solution_from_to[0];
    m_reconstruct_flux->configure_property("from_to",flux_from_to);
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::execute()
{
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex

  Reconstruct& reconstruct_solution_in_flux_points = *m_reconstruct_solution;
  Reconstruct& reconstruct_flux_in_solution_points = *m_reconstruct_flux;

  Flux& compute_flux = *m_flux;

  const Uint dimensionality = elements().element_type().dimensionality();
  const SFDM::ShapeFunction& solution_sf = elements().space("solution").shape_function().as_type<SFDM::ShapeFunction>();
  const SFDM::ShapeFunction& flux_sf     = elements().space("flux")    .shape_function().as_type<SFDM::ShapeFunction>();


  RealMatrix solution(solution_sf.nb_nodes_per_line(),m_solution->field().data().row_size());
  RealMatrix flux(flux_sf.nb_nodes_per_line(),m_solution->field().data().row_size());
  RealMatrix flux_grad(solution_sf.nb_nodes_per_line(),m_solution->field().data().row_size());

  CMultiStateFieldView::View solution_data = (*m_solution)[idx()];
  CMultiStateFieldView::View residual_data = (*m_residual)[idx()];

  /// For every orientation
  for (Uint orientation = KSI; orientation<dimensionality; ++orientation)
  {
    /// For every line in this orientation
    for (Uint line=0; line<solution_sf.nb_lines_per_orientation(); ++line)
    {
      /// Set solution states
      for (Uint point=0; point<solution_sf.nb_nodes_per_line(); ++point)
      {
        solution.row(point) = to_row_vector( solution_data[ solution_sf.points()[orientation][line][point] ] );
      }
      CFinfo << "solution = \n" << solution << CFendl;

      /// Compute flux in flux points
      flux = compute_flux( reconstruct_solution_in_flux_points.value( solution ) );
      CFinfo << "flux = \n" << flux << CFendl;

      /// Update face flux points with Riemann problem with neighbor
      for (Uint side=LEFT; side<=RIGHT; ++side)
      {
        CFinfo << "face_flux["<<side<<"] = " << flux.row( flux_sf.face_points()[orientation][line][side] );
        CFinfo << "   <-->   must solve Riemann problem with neighbor face_flux on side " << side << CFendl;
        /// @todo reconstruct neighbor line and compute Riemann problem at face point
      }

      /// Compute gradient of flux in solution points
      flux_grad = reconstruct_flux_in_solution_points.gradient( flux , static_cast<CoordRef>(orientation) );
      CFinfo << "flux_grad = \n" << flux_grad << CFendl;

      /// Add the flux gradient to the RHS
      for (Uint point=0; point<solution_sf.nb_nodes_per_line(); ++point)
      {
        for (Uint var=0; var<m_residual->field().data().row_size(); ++var)
          residual_data[ solution_sf.points()[orientation][line][point] ][var] -= flux_grad(point,var);
      }

    }
  }


  /// For fun, copy residual into solution but scaled wrongly as there is no jacobian multiplication.
  /// We should now have the gradient in the solution, since flux = solution
  solution_data = residual_data;
}

////////////////////////////////////////////////////////////////////////////////

RealRowVector ComputeRhsInCell::to_row_vector(Mesh::CTable<Real>::ConstRow row) const
{
  RealRowVector rowvec (row.size());
  for (Uint i=0; i<row.size(); ++i)
  {
    rowvec[i] = row[i];
  }
  return rowvec;
}

////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

