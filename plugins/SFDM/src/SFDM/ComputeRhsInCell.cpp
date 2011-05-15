// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/CConnectivity.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"

#include "Solver/State.hpp"

#include "RiemannSolvers/src/RiemannSolvers/RiemannSolver.hpp"

#include "SFDM/ComputeRhsInCell.hpp"
#include "SFDM/Reconstruct.hpp"
#include "SFDM/Flux.hpp"
#include "SFDM/ShapeFunction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::RiemannSolvers;

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

  m_properties.add_option(OptionURI::create("wave_speed","Wave Speed","Wave speed to be calculated. Used for stability condition.", URI("cpath:"),URI::Scheme::CPATH))
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeRhsInCell::config_wavespeed,   this ) )
    ->add_tag("wave_speed");


  properties().add_option( OptionT<std::string>::create("riemann_solver","Riemann Solver","The component to solve the Rieman Problem on cell-faces","CF.RiemannSolvers.Roe") )
      ->mark_basic()
      ->attach_trigger ( boost::bind ( &ComputeRhsInCell::build_riemann_solver, this) )
      ->add_tag("riemann_solver");

  m_properties.add_option( OptionComponent<Solver::State>::create("solution_state","Solution State","The component describing the solution state",&m_sol_state) )
      ->add_tag("solution_state");


  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &ComputeRhsInCell::trigger_elements,   this ) );

  m_solution   = create_static_component_ptr<CMultiStateFieldView>("solution_view");
  m_residual   = create_static_component_ptr<CMultiStateFieldView>("residual_view");
  m_wave_speed = create_static_component_ptr<CScalarFieldView>    ("wave_speed_view");

  m_reconstruct_solution = create_static_component_ptr<Reconstruct>("solution_reconstruction_helper");
  m_reconstruct_flux     = create_static_component_ptr<Reconstruct>("flux_reconstruction_helper");

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

void ComputeRhsInCell::config_wavespeed()
{
  URI uri;
  property("wave_speed").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr_checked(uri)->as_ptr_checked<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");

  m_wave_speed->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::trigger_elements()
{
  m_can_start_loop = m_solution->set_elements(elements());
  m_can_start_loop = m_residual->set_elements(elements());
  m_can_start_loop = m_wave_speed->set_elements(elements());

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

void ComputeRhsInCell::build_riemann_solver()
{
  if (is_not_null(m_riemann_solver))
    remove_component(*m_riemann_solver);
  m_riemann_solver = build_component("riemann_solver",property("riemann_solver").value<std::string>()).as_ptr<RiemannSolver>();
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

  Solver::State& sol_state = *m_sol_state.lock();
  Solver::Physics sol_vars = sol_state.create_physics();
  /// @todo give normal a correct value
  RealVector normal(1); normal << 1.;

  const Uint dimensionality = elements().element_type().dimensionality();
  const SFDM::ShapeFunction& solution_sf = elements().space("solution").shape_function().as_type<SFDM::ShapeFunction>();
  const SFDM::ShapeFunction& flux_sf     = elements().space("flux")    .shape_function().as_type<SFDM::ShapeFunction>();

  const Uint nb_vars = m_solution->field().data().row_size();
  RealMatrix flux_in_line (flux_sf.nb_nodes_per_line() , nb_vars);
  RealMatrix flux_grad_in_line (solution_sf.nb_nodes_per_line() , nb_vars);

  CMultiStateFieldView::View solution_data = (*m_solution)[idx()];
  CMultiStateFieldView::View residual_data = (*m_residual)[idx()];
  Real& wave_speed = (*m_wave_speed)[idx()];

  CConnectivity& c2f = elements().get_child("face_connectivity").as_type<CConnectivity>();
  Component::Ptr faces;
  Uint face_idx;
  Component::Ptr neighbor_cells;
  Uint neighbor_cell_idx;
  const Uint this_cell_idx = m_mesh_elements.lock()->unified_idx(elements(),idx());

  //CFinfo << "\ncell " << idx() << CFendl;
  //CFinfo <<   "------"<<CFendl;
  /// <li> Set all cell solution states in a matrix @f$ \mathbf{Q_s} @f$ (rows are states, columns are variables)
  RealMatrix solution = reconstruct_solution_in_flux_points.value( to_matrix(solution_data) );
  //CFinfo << "solution in flux points = " << solution.transpose() << CFendl;
  /// <li> Compute analytical flux in all flux points (every row is a flux)
  /// <ul>
  ///   <li> First the solution is reconstructed in the flux points.
  ///        @f[ \tilde{Q}(\xi,\eta) = \sum_{s=1}^{N} \tilde{Q}_{s}\  L_{s}(\xi,\eta)@f]
  ///        SFDM::Reconstruct::value() provides a precalculated matrix @f$R@f$ with element (f,s) corresponding to @f$L_{s}(\xi_f,\eta_f)@f$.
  ///        @f[ \mathbf{\tilde{Q}_f} = R \ \mathbf{\tilde{Q}_s} @f]
  ///   <li> Then the analytical flux is calculated in these flux points.
  ///        @f[ \mathbf{\tilde{F}_f} = \mathrm{flux}(\mathbf{\tilde{Q}_f}) @f] (see SFDM::Flux)
  /// </ul>
  RealVector flux(nb_vars);

  /// <li> Compute flux gradients in the solution points, and add to the RHS
  // For every orientation
  for (Uint orientation = KSI; orientation<dimensionality; ++orientation)
  { /// <ul>
    /// <li> For every 1D line of every orientation
    for (Uint line=0; line<solution_sf.nb_lines_per_orientation(); ++line)
    { /// <ul>

      /// <li> Compute flux in line, excluding begin and end point
      for (Uint flux_pt=1; flux_pt<flux_in_line.rows()-1; ++flux_pt)
      {
        sol_state.set_state(solution.row(flux_sf.points()[orientation][line][flux_pt]),sol_vars);
        sol_state.compute_flux(sol_vars,normal,flux);
        flux_in_line.row(flux_pt) = flux;
      }

      /// <li> Update face flux points with Riemann problem with neighbor
      ///      At the flux point location of the face:
      ///      @f[ \tilde{F}_{face flxpt} = \mathrm{Riemann}(Q_{face flxpt,\mathrm{left}},Q_{face flxpt,\mathrm{right}}) @f]
      for (Uint side=0; side<2; ++side) // a line connects 2 faces
      {
        /// @todo 2D and 3D support
        /// It is now assumed that side == face number (only valid in 1D)
        cf_assert_desc("only valid in 1D",dimensionality==1);

        // Find face
        boost::tie(faces,face_idx) = c2f.lookup().location( c2f[idx()][side] );

        // Find neighbor cell
        CFaceCellConnectivity& f2c = faces->get_child("cell_connectivity").as_type<CFaceCellConnectivity>();
        if (f2c.is_bdry_face()[face_idx])
        {
          //CFinfo <</* "cell["<<idx()<<"] */"must implement a boundary condition on face " << faces->parent().name() << "["<<face_idx<<"]" << CFendl;
          /// @todo implement boundary condition
        }
        else
        {
          CTable<Uint>::ConstRow connected_cells = f2c.connectivity()[face_idx];
          Uint unified_neighbor_cell_idx = connected_cells[LEFT] != this_cell_idx ? connected_cells[LEFT] : connected_cells[RIGHT];
          boost::tie(neighbor_cells,neighbor_cell_idx) = f2c.lookup().location( unified_neighbor_cell_idx );

          //CFinfo << /*"cell["<<idx()<<"] */"must solve Riemann problem on face " << faces->parent().name() << "["<<face_idx<<"]  with cell["<<neighbor_cell_idx<<"]" << CFendl;

          /// @todo Multi-region support.
          /// It is now assumed for reconstruction that neighbor_cells == elements(), so that the same field_view "m_solution" can be used.
          cf_assert_desc("does not support multi_region yet",neighbor_cells == elements().self());
          RealMatrix neighbor_solution = reconstruct_solution_in_flux_points.value( to_matrix( (*m_solution)[neighbor_cell_idx] ) );

          RealRowVector left  = solution         .row ( flux_sf.face_points()[orientation][line][side] );
          RealRowVector right = neighbor_solution.row ( flux_sf.face_points()[orientation][line][!side] ); // the other side

          RealVector normal(1); normal << 1.;
          Real left_wave_speed;
          Real right_wave_speed;
          RealVector H(1);
          if (side == 0)
          {
            riemann_solver().solve(left,right, -normal,   H,left_wave_speed,right_wave_speed);
            flux_in_line.topRows<1>() = -H;
          }
          else
          {
            riemann_solver().solve(left,right,  normal,   H,left_wave_speed,right_wave_speed);
            flux_in_line.bottomRows<1>() = H;
          }
          //CFinfo << "   solve Riemann("<<left<<","<<right<<") = " << H << CFendl;

          /// @todo wave speeds storage
          const Real area = 1.;
          const Real wave_speed_contribution = - std::min(left_wave_speed ,0.) * area;
          //CFinfo << "wave_speed_contribution["<<side<<"] = " << wave_speed_contribution << CFendl;

          wave_speed += wave_speed_contribution;
          // Kris:
          //jacobXIntCoef*
          //          m_updateVarSet->getMaxAbsEigenValue(m_pData,m_unitNormalFlxPnts[iFlx]);

        }
      }

      /// <li> Compute gradient of flux in solution points in this line
      ///
      /// @f[\left.\frac{\partial \tilde{F}}{\partial \xi}\right|_{line,solpt}  = \sum_{fluxpt=1}^{N_f} \tilde{F}_{line,flxpt} \ l'_{flxpt}(\xi_{solpt}) @f]
      /// with @f$ N_f @f$ the number of flux points in the flux 1D shape function.
      ///
      /// This is implemented using SFDM::Reconstruct::gradient()
      flux_grad_in_line = reconstruct_flux_in_solution_points.gradient( flux_in_line , static_cast<CoordRef>(orientation) );
      //CFinfo << "flux_grad_in_line = " << flux_grad_in_line.transpose() << CFendl;

      /// <li> Add the flux gradient to the RHS
      for (Uint point=0; point<solution_sf.nb_nodes_per_line(); ++point)
      {
        for (Uint var=0; var<nb_vars; ++var)
          residual_data[ solution_sf.points()[orientation][line][point] ][var] -= flux_grad_in_line(point,var);
      }
    } /// </ul>
  } /// </ul>
  /// </ul>

  //CFinfo << "wave_speed = " << wave_speed << CFendl;
  /// @section _ideas_for_efficiency Ideas for efficiency
  /// - store Riemann fluxes in face flux points
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

