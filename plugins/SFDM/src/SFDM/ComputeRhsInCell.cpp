// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/CConnectivity.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"

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

  m_mesh_elements = m_solution->field().parent().as_type<CMesh>().elements().as_ptr<CMeshElements>();
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
    flux_from_to[0] = elements().space("flux").shape_function().as_type<SFDM::ShapeFunction>().line().derived_type_name();
    flux_from_to[1] = elements().space("solution").shape_function().as_type<SFDM::ShapeFunction>().line().derived_type_name();
    m_reconstruct_flux->configure_property("from_to",flux_from_to);
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::execute()
{
  /// @section _theory Theory
  /// A general 2D non-linear convection equation is given:
  /// @f[ \frac{\partial \tilde{Q}}{\partial t} + \frac{\partial \tilde{F}}{\partial \xi} + \frac{\partial \tilde{G}}{\partial \eta} = 0 @f]
  /// The solution @f$Q@f$ can be described using the solution shapefunction:
  /// @f[\tilde{Q}(\xi,\eta) = \sum_{s=1}^{N} \tilde{Q}_{s}\  L_{s}(\xi,\eta)@f]
  /// with @f$ N @f$ the total number of solution points in the entire cell.
  /// It has been proved by Kris Van den Abeele that only line (1D), quadrilateral (2D), and hexahedral (3D) cells provide
  /// stable schemes up to any order. The solution shape functions consist of a tensor product of 1D shape functions in every direction.
  /// @f[ L_s(\xi,\eta) = h_i(\xi) \ h_j(\eta) @f]
  /// with @f$ i @f$ the index along the @f$ \xi @f$ axis of solution point @f$ s @f$, and
  /// @f$ j @f$ the index along the @f$ \eta @f$ axis of solution point @f$ s @f$,
  /// and the 1D shapefunction defined as @f[ h_r(X) = \prod_{s=0,s \neq r}^{N_s} \frac{X-X_s}{X_r-X_s} @f]
  /// The solution can be reformulated:
  /// @f[\tilde{Q}(\xi,\eta) = \sum_{i=1}^{N_s} \sum_{j=1}^{N_s} \tilde{Q}_{ij}\  h_i(\xi) \ h_j(\eta)@f]
  /// @f$ N_s @f$ is now the number of solution points along a 1D shape function line.
  ///
  /// For the fluxes a shape function of 1 order higher than the solution shape function is used in the flux direction, so that @f$\frac{\partial F}{\partial \xi}@f$
  /// will be of the same order as the solution @f$ Q @f$
  /// @f[\tilde{F}(\xi,\eta) = \sum_{i=1}^{N_f} \sum_{j=1}^{N_s} \tilde{F}_{ij} \ l_i(\xi) \ h_j(\eta) @f]
  /// @f[\tilde{G}(\xi,\eta) = \sum_{i=1}^{N_s} \sum_{j=1}^{N_f} \tilde{F}_{ij} \ h_i(\xi) \ l_j(\eta) @f]
  /// with @f$ l_r @f$ the 1D shape function for the flux defined as
  /// @f[ l_r(X) = \prod_{f=0,f \neq r}^{N_f} \frac{X-X_f}{X_r-X_f} @f]
  /// The derivatives of the fluxes are then:
  /// @f[\frac{\partial \tilde{F}}{\partial \xi}(\xi,\eta) = \sum_{i=1}^{N_f} \sum_{j=1}^{N_s} \tilde{F}_{ij} \ l'_i(\xi) \ h_j(\eta) @f]
  /// @f[\frac{\partial \tilde{G}}{\partial \eta}(\xi,\eta) = \sum_{i=1}^{N_s} \sum_{j=1}^{N_f} \tilde{F}_{ij} \ h_i(\xi) \ l'_j(\eta) @f]
  /// Evaluating in a @a solution @a point I,J simplifies these equations, as @f$ h_j(\eta_J) = \delta_{jJ} @f$
  /// @f[\left.\frac{\partial \tilde{F}}{\partial \xi}\right|_{IJ}  = \sum_{i=1}^{N_f} \tilde{F}_{iJ} \ l'_i(\xi_I) @f]
  /// @f[\left.\frac{\partial \tilde{G}}{\partial \eta}\right|_{IJ} = \sum_{j=1}^{N_f} \tilde{G}_{Ij} \ l'_j(\eta_J) @f]
  /// This means that the flux gradients can be reconstructed in the solution points, using only the flux values along a 1D line.

  /// @section _algorithm Algorithm per cell
  /// <ul>
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex

  Reconstruct& reconstruct_solution_in_flux_points = *m_reconstruct_solution;
  Reconstruct& reconstruct_flux_in_solution_points = *m_reconstruct_flux;

  Flux& compute_flux = *m_flux;

  const Uint dimensionality = elements().element_type().dimensionality();
  const SFDM::ShapeFunction& solution_sf = elements().space("solution").shape_function().as_type<SFDM::ShapeFunction>();
  const SFDM::ShapeFunction& flux_sf     = elements().space("flux")    .shape_function().as_type<SFDM::ShapeFunction>();

  const Uint nb_vars = m_solution->field().data().row_size();
  RealMatrix flux_in_line (flux_sf.nb_nodes_per_line() , nb_vars);
  RealMatrix flux_grad_in_line (solution_sf.nb_nodes_per_line() , nb_vars);

  CMultiStateFieldView::View solution_data = (*m_solution)[idx()];
  CMultiStateFieldView::View residual_data = (*m_residual)[idx()];

  CConnectivity& c2f = elements().get_child("face_connectivity").as_type<CConnectivity>();
  Component::Ptr faces;
  Uint face_idx;
  Component::Ptr neighbor_cells;
  Uint neighbor_cell_idx;
  const Uint this_cell_idx = m_mesh_elements.lock()->unified_idx(elements(),idx());

  /// <li> Set all cell solution states in a matrix @f$ \mathbf{Q_s} @f$ (rows are states, columns are variables)
  RealMatrix solution = to_matrix(solution_data);
  CFinfo << "solution = \n" << solution << CFendl;

  /// <li> Compute analytical flux in all flux points (every row is a flux)
  /// <ul>
  ///   <li> First the solution is reconstructed in the flux points.
  ///        @f[ \tilde{Q}(\xi,\eta) = \sum_{s=1}^{N} \tilde{Q}_{s}\  L_{s}(\xi,\eta)@f]
  ///        SFDM::Reconstruct::value() provides a precalculated matrix @f$R@f$ with element (f,s) corresponding to @f$L_{s}(\xi_f,\eta_f)@f$.
  ///        @f[ \mathbf{\tilde{Q}_f} = R \ \mathbf{\tilde{Q}_s} @f]
  ///   <li> Then the analytical flux is calculated in these flux points.
  ///        @f[ \mathbf{\tilde{F}_f} = \mathrm{flux}(\mathbf{\tilde{Q}_f}) @f] (see SFDM::Flux)
  /// </ul>
  RealMatrix flux = compute_flux( reconstruct_solution_in_flux_points.value( solution ) );
  CFinfo << "flux = \n" << flux << CFendl;

  /// <li> Compute flux gradients in the solution points, and add to the RHS
  // For every orientation
  for (Uint orientation = KSI; orientation<dimensionality; ++orientation)
  { /// <ul>
    /// <li> For every 1D line of every orientation
    for (Uint line=0; line<solution_sf.nb_lines_per_orientation(); ++line)
    { /// <ul>
      /// <li> Update face flux points with Riemann problem with neighbor
      ///      At the flux point location of the face:
      ///      @f[ \tilde{F}_{face flxpt} = \mathrm{Riemann}(Q_{face flxpt,\mathrm{left}},Q_{face flxpt,\mathrm{right}}) @f]
      for (Uint side=0; side<2; ++side) // a line connects 2 faces
      {
        /// @todo it is now assumed that side == face number (only valid in 1D)
        cf_assert_desc("only valid in 1D",dimensionality==1);

        // Find face
        boost::tie(faces,face_idx) = c2f.lookup().location( c2f[idx()][side] );

        // Find neighbor cell
        CFaceCellConnectivity& f2c = faces->get_child("cell_connectivity").as_type<CFaceCellConnectivity>();
        if (f2c.is_bdry_face()[face_idx])
        {
          CFinfo << "cell["<<idx()<<"] must implement a boundary condition on face " << faces->full_path().path() << "["<<face_idx<<"]" << CFendl;
          /// @todo implement boundary condition
        }
        else
        {
          CTable<Uint>::ConstRow connected_cells = f2c.connectivity()[face_idx];
          Uint unified_neighbor_cell_idx = connected_cells[LEFT] != this_cell_idx ? connected_cells[LEFT] : connected_cells[RIGHT];
          boost::tie(neighbor_cells,neighbor_cell_idx) = f2c.lookup().location( unified_neighbor_cell_idx );

          CFinfo << "cell["<<idx()<<"] must solve Riemann problem on face " << faces->full_path().path() << "["<<face_idx<<"]  with cell["<<neighbor_cell_idx<<"]" << CFendl;
          /// @todo reconstruct solution from neighbor cell and solve Riemann problem
        }
      }

      /// <li> Compute gradient of flux in solution points in this line
      ///
      for (Uint flux_pt=0; flux_pt<flux_in_line.rows(); ++flux_pt)
        flux_in_line.row(flux_pt) = flux.row( flux_sf.points()[orientation][line][flux_pt] );

     /// @f[\left.\frac{\partial \tilde{F}}{\partial \xi}\right|_{line,solpt}  = \sum_{fluxpt=1}^{N_f} \tilde{F}_{line,flxpt} \ l'_{flxpt}(\xi_{solpt}) @f]
     /// with @f$ N_f @f$ the number of flux points in the flux 1D shape function.
     ///
     /// This is implemented using SFDM::Reconstruct::gradient()
     flux_grad_in_line = reconstruct_flux_in_solution_points.gradient( flux_in_line , static_cast<CoordRef>(orientation) );
      CFinfo << "flux_grad_in_line = \n" << flux_grad_in_line << CFendl;

      /// <li> Add the flux gradient to the RHS
      for (Uint point=0; point<solution_sf.nb_nodes_per_line(); ++point)
      {
        for (Uint var=0; var<nb_vars; ++var)
          residual_data[ solution_sf.points()[orientation][line][point] ][var] -= flux_grad_in_line(point,var);
      }
    } /// </ul>
  } /// </ul>
  /// </ul>

  /// For fun, copy residual into solution but scaled wrongly as there is no jacobian multiplication.
  /// We should now have the gradient in the solution, since flux = solution (see SFDM::Flux)
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

RealMatrix ComputeRhsInCell::to_matrix(Mesh::CMultiStateFieldView::View data) const
{
  RealMatrix m (data.shape()[0] , data.shape()[1]);
  for (Uint i=0; i<m.rows(); ++i)
    for (Uint j=0; j<m.cols(); ++j)
      m(i,j)=data[i][j];
  return m;
}


////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

