// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"

#include "common/Builder.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "math/MathConsts.hpp"

#include "mesh/CField.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/FaceCellConnectivity.hpp"

#include "solver/State.hpp"

#include "RiemannSolvers/src/RiemannSolvers/RiemannSolver.hpp"

#include "SFDM/ComputeRhsInCell.hpp"
#include "SFDM/Reconstruct.hpp"
#include "SFDM/ShapeFunction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::RiemannSolvers;
using namespace cf3::math::MathConsts;

namespace cf3 {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeRhsInCell, solver::Actions::CLoopOperation, LibSFDM > ComputeRhsInCell_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeRhsInCell::ComputeRhsInCell ( const std::string& name ) :
  solver::Actions::CLoopOperation(name)
{
  // options
  m_options.add_option(OptionURI::create("solution", URI("cpath:"), URI::Scheme::CPATH))
    ->description("Solution to calculate RHS for")
    ->pretty_name("Solution")
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeRhsInCell::config_solution,   this ) );

  m_options.add_option(OptionURI::create("residual", URI("cpath:"),URI::Scheme::CPATH))
    ->description("Residual to be calculated")
    ->pretty_name("Residual")
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeRhsInCell::config_residual,   this ) );

  m_options.add_option(OptionURI::create("wave_speed", URI("cpath:"),URI::Scheme::CPATH))
    ->description("Wave speed to be calculated. Used for stability condition.")
    ->pretty_name("Wave Speed")
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeRhsInCell::config_wavespeed,   this ) );

  m_options.add_option(OptionURI::create("jacobian_determinant", URI("cpath:"),URI::Scheme::CPATH))
    ->description("Jacobian Determinant of the Transformation to mapped space")
    ->pretty_name("Jacobian Determinant")
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeRhsInCell::config_jacobian_determinant,   this ) );

  m_options.add_option( OptionT<std::string>::create("riemann_solver", "cf3.RiemannSolvers.Roe") )
    ->description("The component to solve the Rieman Problem on cell-faces")
    ->pretty_name("Riemann Solver")
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &ComputeRhsInCell::build_riemann_solver, this) );

  m_options.add_option( OptionComponent<solver::State>::create("solution_state", &m_sol_state) )
    ->description("The component describing the solution state")
    ->pretty_name("Solution State")
    ->attach_trigger (boost::bind ( &ComputeRhsInCell::config_solution_physics, this) );


  m_options["Elements"].attach_trigger ( boost::bind ( &ComputeRhsInCell::trigger_elements,   this ) );

  m_solution             = create_static_component_ptr<CMultiStateFieldView>("solution_view");
  m_residual             = create_static_component_ptr<CMultiStateFieldView>("residual_view");
  m_jacobian_determinant = create_static_component_ptr<CMultiStateFieldView>("jacobian_determinant_view");
  m_wave_speed           = create_static_component_ptr<CScalarFieldView>    ("wave_speed_view");

  m_reconstruct_solution = create_static_component_ptr<Reconstruct>("solution_reconstruction_helper");
  m_reconstruct_flux     = create_static_component_ptr<Reconstruct>("flux_reconstruction_helper");

}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::config_solution()
{
  URI uri;
  option("solution").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_solution->set_field(comp);

  m_mesh_elements = m_solution->field().parent().as_type<Mesh>().elements().as_ptr<MeshElements>();
  m_nb_vars = m_solution->field().data().row_size();

}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::config_solution_physics()
{
  m_sol_vars = m_sol_state.lock()->create_physics();
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::config_residual()
{
  URI uri;
  option("residual").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_residual->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::config_jacobian_determinant()
{
  URI uri;
  option("jacobian_determinant").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_jacobian_determinant->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::config_wavespeed()
{
  URI uri;
  option("wave_speed").put_value(uri);
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
  m_can_start_loop = m_jacobian_determinant->set_elements(elements());
  m_can_start_loop = m_wave_speed->set_elements(elements());

  if (m_can_start_loop)
  {
    m_solution_sf =  elements().space("solution").shape_function().as_ptr_checked<SFDM::ShapeFunction>();
    m_flux_sf     =  elements().space("flux").shape_function().as_ptr_checked<SFDM::ShapeFunction>();

    std::vector<std::string> solution_from_to(2);
    solution_from_to[0] = m_solution_sf->derived_type_name();
    solution_from_to[1] = m_flux_sf->derived_type_name();
    m_reconstruct_solution->configure_option("from_to",solution_from_to);

    std::vector<std::string> flux_from_to(2);
    flux_from_to[0] = m_flux_sf->line().derived_type_name();
    flux_from_to[1] = m_solution_sf->line().derived_type_name();
    m_reconstruct_flux->configure_option("from_to",flux_from_to);

    m_dimensionality = elements().element_type().dimensionality();
    // Create normals for every orientation
    m_normal.resize(m_dimensionality,RealVector::Zero(m_dimensionality));
    for (Uint orientation = KSI; orientation<m_dimensionality; ++orientation)
      m_normal[orientation][orientation] = 1.;

    flux.resize(m_nb_vars);
    flux_in_line.resize(m_flux_sf->nb_nodes_per_line() , m_nb_vars);
    flux_grad_in_line.resize(m_solution_sf->nb_nodes_per_line() , m_nb_vars);

    solution.resize(m_solution_sf->nb_nodes(),m_nb_vars);
    neighbor_solution.resize(m_solution_sf->nb_nodes(),m_nb_vars);
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::build_riemann_solver()
{
  if (is_not_null(m_riemann_solver))
    remove_component(*m_riemann_solver);
  m_riemann_solver = create_component("riemann_solver",option("riemann_solver").value<std::string>()).as_ptr<RiemannSolver>();
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeRhsInCell::execute()
{
  /// @section _theory Theory
  /// A general 2D non-linear system of hyperbolic equations is given:
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
  /// @f[\tilde{G}(\xi,\eta) = \sum_{i=1}^{N_s} \sum_{j=1}^{N_f} \tilde{G}_{ij} \ h_i(\xi) \ l_j(\eta) @f]
  /// with @f$ l_r @f$ the 1D shape function for the flux defined as
  /// @f[ l_r(X) = \prod_{f=0,f \neq r}^{N_f} \frac{X-X_f}{X_r-X_f} @f]
  /// The derivatives of the fluxes are then:
  /// @f[\frac{\partial \tilde{F}}{\partial \xi}(\xi,\eta) = \sum_{i=1}^{N_f} \sum_{j=1}^{N_s} \tilde{F}_{ij} \ l'_i(\xi) \ h_j(\eta) @f]
  /// @f[\frac{\partial \tilde{G}}{\partial \eta}(\xi,\eta) = \sum_{i=1}^{N_s} \sum_{j=1}^{N_f} \tilde{F}_{ij} \ h_i(\xi) \ l'_j(\eta) @f]
  ///
  /// @subsection _theory_simplification Simplifications
  /// In the case we are interested only in reconstruction in a line inside the element, and the line for the flux and the line for the solution coincide, these shape functions can be simplified.@n
  /// Evaluating the solution in a @a flux @a point I on line J becomes (as @f$ h_j(\eta_J) = \delta_{jJ} @f$)
  /// @f[\tilde{Q}_J(\xi_I)  = \sum_{i=1}^{N_s} \tilde{Q}_{iJ} \ h_i(\xi_I) @f]
  /// Evaluating the solution in a @a flux @a point J on line I becomes (as @f$ h_i(\xi_I) = \delta_{iI} @f$)
  /// @f[\tilde{Q}_I(\eta_J) = \sum_{j=1}^{N_s} \tilde{Q}_{Ij} \ h_j(\eta_J) @f]
  /// Evaluating the flux in a @a solution @a point I on line J becomes (as @f$ h_j(\eta_J) = \delta_{jJ} @f$)
  /// @f[\tilde{F}_J(\xi_I) = \sum_{i=1}^{N_f} \tilde{F}_{iJ} \ l_i(\xi_I) @f]
  /// Evaluating the flux in a @a solution @a point J on line I becomes (as @f$ h_i(\xi_I) = \delta_{iI} @f$)
  /// @f[\tilde{G}_I(\eta_J) = \sum_{j=1}^{N_f} \tilde{G}_{Ij} \ l_j(\eta_J) @f]
  /// Evaluating the flux gradient in a @a solution @a point I on line J becomes (as @f$ h_j(\eta_J) = \delta_{jJ} @f$)
  /// @f[\left.\frac{\partial \tilde{F}}{\partial \xi}\right|_{J}(\xi_I)  = \sum_{i=1}^{N_f} \tilde{F}_{iJ} \ l'_i(\xi_I) @f]
  /// Evaluating the flux gradient in a @a solution @a point J on line I becomes (as @f$ h_i(\xi_I) = \delta_{iI} @f$)
  /// @f[\left.\frac{\partial \tilde{G}}{\partial \eta}\right|_{I}(\eta_J) = \sum_{j=1}^{N_f} \tilde{G}_{Ij} \ l'_j(\eta_J) @f]
  /// @n
  /// This means that the flux values and gradients can be reconstructed in the solution points, using only the flux values along a 1D line.

  /// @section _algorithm Algorithm per cell
  /// Definition of RHS: @f$ \frac{\partial Q}{\partial t} = - \frac{1}{|J|} \ \nabla \cdot \tilde{F}@f$ @n
  /// <ul>
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex

  Reconstruct& reconstruct_solution_in_all_flux_points = *m_reconstruct_solution;
  Reconstruct& reconstruct_flux_in_solution_points_in_line = *m_reconstruct_flux;

  solver::State& sol_state = *m_sol_state.lock();
  solver::Physics& sol_vars = *m_sol_vars;

  const SFDM::ShapeFunction& solution_sf = *m_solution_sf;
  const SFDM::ShapeFunction& flux_sf     = *m_flux_sf;

  const ElementType&   geometry   = elements().element_type();
  RealMatrix geometry_coords  = elements().get_coordinates( idx() );

  CMultiStateFieldView::View solution_data = (*m_solution)[idx()];
  CMultiStateFieldView::View residual_data = (*m_residual)[idx()];
  CMultiStateFieldView::View jacobian_determinant_data = (*m_jacobian_determinant)[idx()];

  RealMatrix solution_in_sol_pts = to_matrix(solution_data);
  RealVector jacobian_determinant = to_matrix(jacobian_determinant_data);  // RealVector is column shaped RealMatrix

  Real& wave_speed = (*m_wave_speed)[idx()];

  Connectivity& c2f = elements().get_child("face_connectivity").as_type<Connectivity>();
  Component::Ptr faces;
  Uint face_idx;
  Component::Ptr neighbor_cells;
  Uint neighbor_cell_idx;
  const Uint this_cell_idx = m_mesh_elements.lock()->unified_idx(elements(),idx());

  //CFdebug << "\ncell " << idx() << CFendl;
  //CFdebug <<   "------"<<CFendl;
  /// <li> Set all cell solution states in a matrix @f$ \mathbf{Q_s} @f$ (rows are states, columns are variables)@n
  /// <li> Reconstruct the solution in the flux points.
  ///      @f[ \tilde{Q}(\xi,\eta) = \sum_{s=1}^{N} |J_s| {Q}_{s}\  L_{s}(\xi,\eta)@f]
  ///      SFDM::Reconstruct::value() provides a precalculated matrix @f$R@f$ with element (f,s) corresponding to @f$L_{s}(\xi_f,\eta_f)@f$.
  ///      @f[ \mathbf{\tilde{Q}_f} = R \ \mathbf{\tilde{Q}_s} @f]
  ///      where subscript @f$ _f @f$ denotes the values in the flux points.
  solution = reconstruct_solution_in_all_flux_points.value( to_matrix(solution_data) ) ;
  //CFdebug << "rhs = \n" << to_matrix(residual_data) << CFendl;
  //CFdebug << "solution = \n" << to_matrix(solution_data) << CFendl;
  //CFdebug << "mapped solution in flux points = \n" << solution << CFendl;

  max_wave_speed = 0.;


  /// <li> Compute flux gradients in the solution points, and add to the RHS
  // For every orientation
  for (Uint orientation = KSI; orientation<m_dimensionality; ++orientation)
  { /// <ul>
    /// <li> For every 1D line of every orientation
    //CFdebug << "orientation = " << orientation << CFendl;
    for (Uint line=0; line<solution_sf.nb_lines_per_orientation(); ++line)
    { /// <ul>
      //CFdebug << "  line = " << line << CFendl;
      /// <li> Compute analytical flux in the flux points of the line, excluding begin and end point
      ///      @f[ \mathbf{\tilde{F}}_{f,line} = \mathrm{flux}(\mathbf{\tilde{Q}}_{f, line}) @f] (see solver::State::compute_flux())

      for (Uint sol_pt=0; sol_pt<solution_sf.nb_nodes_per_line(); ++sol_pt)
      {
        sol_state.set_state(solution_in_sol_pts.row(solution_sf.points()[orientation][line][sol_pt]),sol_vars);
        RealVector plane_area_normal = geometry.plane_jacobian_normal(solution_sf.local_coordinates().row(solution_sf.points()[orientation][line][sol_pt]), geometry_coords , (cf3::CoordRef) orientation);
        for (Uint i=0; i<m_dimensionality-1; ++i)
          plane_area_normal = plane_area_normal * 2.;
        max_wave_speed = std::max(max_wave_speed, sol_state.max_abs_eigen_value(sol_vars, plane_area_normal ) );// / jacobian_determinant[ solution_sf.points()[orientation][line][sol_pt] ] );
      }

      for (Uint flux_pt=1; flux_pt<flux_in_line.rows()-1; ++flux_pt)
      {
        //RealMatrix jacobian = geometry.jacobian(flux_sf.local_coordinates().row(flux_sf.points()[orientation][line][flux_pt]),geometry_coords);
        sol_state.set_state(solution.row(flux_sf.points()[orientation][line][flux_pt]),sol_vars);
        //sol_state.compute_flux(sol_vars,m_normal[orientation],flux);
        sol_state.compute_flux(sol_vars, geometry.plane_jacobian_normal(flux_sf.local_coordinates().row(flux_sf.points()[orientation][line][flux_pt]), geometry_coords , (cf3::CoordRef) orientation) ,flux);
        flux_in_line.row(flux_pt) = flux;
      }

      /// <li> Update face flux points with Riemann problem with neighbor
      ///      At the flux point location of the face:
      ///      @f[ \tilde{F}_{\mathrm{facepoint}} = \mathrm{Riemann}(\tilde{Q}_{\mathrm{facepoint},\mathrm{left}},\tilde{Q}_{\mathrm{facepoint},\mathrm{right}}) @f]
      for (Uint side=0; side<2; ++side) // a line connects 2 faces
      {
        // Find face
        boost::tie(faces,face_idx) = c2f.lookup().location( c2f[idx()][flux_sf.face_number()[orientation][side]] );

        // Find neighbor cell
        FaceCellConnectivity& f2c = faces->get_child("cell_connectivity").as_type<FaceCellConnectivity>();
        if (f2c.is_bdry_face()[face_idx])
        {
          //CFdebug << "    must implement a boundary condition on face " << faces->parent().name() << "["<<face_idx<<"]" << CFendl;
          /// @todo implement real boundary condition.
          if (side == 0)
          {
            sol_state.set_state(solution.row(flux_sf.points()[orientation][line][0]),sol_vars);
            sol_state.compute_flux(sol_vars, geometry.plane_jacobian_normal(flux_sf.local_coordinates().row(flux_sf.face_points()[orientation][line][0]), geometry_coords , (cf3::CoordRef) orientation) ,flux);
            flux_in_line.topRows<1>() = flux;
          }
          else
          {
            sol_state.set_state(solution.row(flux_sf.face_points()[orientation][line][1]),sol_vars);
            sol_state.compute_flux(sol_vars, geometry.plane_jacobian_normal(flux_sf.local_coordinates().row(flux_sf.face_points()[orientation][line][1]), geometry_coords , (cf3::CoordRef) orientation) ,flux);
            flux_in_line.bottomRows<1>() = flux;
          }
        }
        else
        {
          Table<Uint>::ConstRow connected_cells = f2c.connectivity()[face_idx];
          Uint unified_neighbor_cell_idx = connected_cells[LEFT] != this_cell_idx ? connected_cells[LEFT] : connected_cells[RIGHT];
          boost::tie(neighbor_cells,neighbor_cell_idx) = f2c.lookup().location( unified_neighbor_cell_idx );

          //CFdebug << "    must solve Riemann problem on face " << faces->parent().name() << "["<<face_idx<<"]  with cell["<<neighbor_cell_idx<<"]" << CFendl;

          /// @todo Multi-region support.
          /// It is now assumed for reconstruction that neighbor_cells == elements(), so that the same field_view "m_solution" can be used.
          cf_assert_desc("does not support multi_region yet",neighbor_cells == elements().self());
          neighbor_solution = reconstruct_solution_in_all_flux_points.value( to_matrix( (*m_solution)[neighbor_cell_idx] ) );

          const RealRowVector& left  = solution         .row ( flux_sf.face_points()[orientation][line][side] );
          const RealRowVector& right = neighbor_solution.row ( flux_sf.face_points()[orientation][line][!side] ); // the other side


          if (side == 0)
          {
            riemann_solver().solve(left,right, -geometry.plane_jacobian_normal(flux_sf.local_coordinates().row(flux_sf.face_points()[orientation][line][0]), geometry_coords, (cf3::CoordRef) orientation),   flux,left_wave_speed,right_wave_speed);
            flux_in_line.topRows<1>() = -flux;
          }
          else
          {
            riemann_solver().solve(left,right,  geometry.plane_jacobian_normal(flux_sf.local_coordinates().row(flux_sf.points()[orientation][line][1]), geometry_coords, (cf3::CoordRef) orientation),   flux,left_wave_speed,right_wave_speed);
            flux_in_line.bottomRows<1>() = flux;
          }
          //CFdebug << "      solve Riemann("<<left<<","<<right<<") with normal ["<< (side==0?-1.:1.)*geometry.plane_jacobian_vector(flux_sf.local_coordinates().row(flux_sf.face_points()[orientation][line][side]), geometry_coords, (cf3::CoordRef) orientation).transpose()<<"] = \n" << flux << CFendl;

          for (Uint i=0; i<m_dimensionality-1; ++i)
            left_wave_speed = std::abs(left_wave_speed) * 2.;
          max_wave_speed = std::max(max_wave_speed, std::abs(left_wave_speed) );

        }
      }

      //CFdebug << "    flux_in_line = \n" << flux_in_line << CFendl;

      /// <li> Compute gradient of flux in solution points in this line
      ///
      /// @f[\left.\frac{\partial \tilde{F}}{\partial \xi}\right|_{line,s}  = \sum_{f=1}^{N_f} \tilde{F}_{line,f} \ l'_{f}(\xi_{s}) @f]
      /// with @f$ N_f @f$ the number of flux points in the flux 1D shape function.
      ///
      /// This is implemented using SFDM::Reconstruct::gradient()
      flux_grad_in_line = reconstruct_flux_in_solution_points_in_line.gradient( flux_in_line , KSI ); // KSI because line has only 1 orientation
      //CFdebug << "    flux_grad_in_line = \n" << flux_grad_in_line << CFendl;

      /// <li> Add the flux gradient to the RHS
      ///      @f[ \frac{\partial Q_s}{\partial t} = - \frac{1}{|J_s|} \ \nabla_{\mathbf{\xi}} \cdot {\tilde{F}_s} @f]
      for (Uint point=0; point<solution_sf.nb_nodes_per_line(); ++point)
      {
        const Uint solution_idx = solution_sf.points()[orientation][line][point];
        //CFdebug << "    jacobian_determinant["<<point<<"] = " << jacobian_determinant[solution_idx] << CFendl;
        for (Uint var=0; var<m_nb_vars; ++var)
          residual_data[solution_idx][var] -= flux_grad_in_line(point,var) / jacobian_determinant[solution_idx];
      }
    } /// </ul>
  } /// </ul>
  /// </ul>
  wave_speed = max_wave_speed;
  //CFdebug << "wave_speed = " << wave_speed << CFendl;
  //CFdebug << "rhs = \n" << to_matrix(residual_data) << CFendl;
  /// @section _ideas_for_efficiency Ideas for efficiency
  /// - store Riemann fluxes in face flux points
  /// - work only in mapped space, and do transformations only after
}

////////////////////////////////////////////////////////////////////////////////

RealRowVector ComputeRhsInCell::to_row_vector(common::Table<Real>::ConstRow row) const
{
  RealRowVector rowvec (row.size());
  for (Uint i=0; i<row.size(); ++i)
  {
    rowvec[i] = row[i];
  }
  return rowvec;
}

////////////////////////////////////////////////////////////////////////////////////

RealMatrix ComputeRhsInCell::to_matrix(mesh::CMultiStateFieldView::View data) const
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

