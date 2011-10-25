// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"
#include "common/PE/debug.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldGroup.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/CellFaces.hpp"

#include "physics/Variables.hpp"
#include "physics/PhysModel.hpp"

#include "RiemannSolvers/RiemannSolvers/RiemannSolver.hpp"

#include "SFDM/Convection.hpp"
#include "SFDM/ShapeFunction.hpp"
#include "SFDM/Tags.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

  using namespace common;
  using namespace mesh;
  using namespace physics;
  using namespace RiemannSolvers;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Convection, Term, LibSFDM> Convection_builder;

//////////////////////////////////////////////////////////////////////////////

Convection::Convection( const std::string& name )
  : Term(name)
{
  properties()["brief"] = std::string("Convective Spectral Finite Difference term");
  properties()["description"] = std::string("Fields to be created: ...");

  m_options.add_option( OptionT<std::string>::create("riemann_solver", "cf3.RiemannSolvers.Roe") )
    ->description("The component to solve the Rieman Problem on cell-faces")
    ->pretty_name("Riemann Solver")
    ->mark_basic()
    ->attach_trigger ( boost::bind ( &Convection::build_riemann_solver, this) );

  option(SFDM::Tags::physical_model()).attach_trigger( boost::bind ( &Convection::trigger_physical_model, this ) );
}

/////////////////////////////////////////////////////////////////////////////

void Convection::trigger_physical_model()
{
  build_riemann_solver();

  // try to configure solution vars
  if (Component::Ptr found_solution_vars = find_component_ptr_with_tag(physical_model(),SFDM::Tags::solution_vars()) )
    m_solution_vars = found_solution_vars->as_ptr<Variables>();
}

/////////////////////////////////////////////////////////////////////////////

void Convection::build_riemann_solver()
{
  if (is_not_null(m_riemann_solver))
    remove_component(*m_riemann_solver);
  m_riemann_solver = create_component("riemann_solver",option("riemann_solver").value<std::string>()).as_ptr<RiemannSolver>();
  m_riemann_solver->configure_option("physical_model",physical_model().uri());
}

/////////////////////////////////////////////////////////////////////////////

void Convection::execute()
{
  link_fields();

  // Set residual and wave_speeds to zero
  residual().  as_type< Table<Real> >() = 0.;
  wave_speed().as_type< Table<Real> >() = 0.;

//  compute_one_cell_at_a_time();
  compute_cell_interior_flux_points_contribution();
  compute_inner_face_flux_points_contribution();

}

/////////////////////////////////////////////////////////////////////////////

void Convection::compute_one_cell_at_a_time()
{
  Field& residual_field = residual();
  Field& solution_field = solution();
  Field& wave_speed_field = wave_speed();
  Field& jacob_det_field = jacob_det();

  Variables& solution_vars = *m_solution_vars.lock();

  const Uint nb_vars = solution_field.row_size();

  std::auto_ptr<physics::Properties> dummy_props = physical_model().create_properties();
  RealMatrix dummy_flux( physical_model().neqs(), physical_model().ndim() );
  RealVector dummy_coords( physical_model().ndim() );
  RealMatrix dummy_grads( physical_model().neqs(), physical_model().ndim() );
  RealVector dummy_ev(nb_vars);

  /// Cells loop
  boost_foreach(Region::Ptr region, m_loop_regions)
  boost_foreach(Cells& elements, find_components_recursively<Cells>(*region))
  if( field_group().elements_lookup().contains(elements))
  {
    //std::cout << "      ConvectionTerm() for cells " << elements.uri().string() << std::endl;
    const Space& space = field_group().space(elements);
    const SFDM::ShapeFunction& shape_func = space.shape_function().as_type<SFDM::ShapeFunction>();

    const Uint nb_sol_pts = shape_func.line().nb_nodes();
    const Uint nb_lines = shape_func.dimensionality() == 1 ? 1 : nb_sol_pts;
    const Uint nb_flx_pts = shape_func.flux_line().nb_nodes();

//    if (nb_flx_pts == 2)
//    {
//      // In this case, there are no inner flux points, and thus no contribution from a
//      // cell term. The Riemann problem will be solved in the face term
//      return;
//    }

    RealMatrix geometry_nodes;
    elements.allocate_coordinates(geometry_nodes);
    ElementType& geometry = elements.element_type();
    RealVector unit_normal(geometry.dimension());
    RealVector plane_jacobian_normal(geometry.dimension());
    RealVector face_pt_local_coord(geometry.dimensionality());
    RealVector flux_pt_coord(1);
    RealVector sol_pt_coord(1);

    RealMatrix solution_in_flx_pts(nb_flx_pts,nb_vars);
    RealVector wave_speed_in_flx_pts(nb_flx_pts);
    RealMatrix flux_in_flx_pts(nb_flx_pts,nb_vars);

    RealMatrix flux_grad_in_sol_pts(nb_sol_pts, nb_vars);
    RealVector wave_speed_in_sol_pts(nb_sol_pts);
    RealMatrix flux_sf_gradient(1,nb_flx_pts);
    RealRowVector flux_sf_value(nb_flx_pts);

    RealMatrix sol_to_flux_pts(nb_flx_pts, nb_sol_pts);
    for (Uint flx_pt=0; flx_pt<nb_flx_pts; ++flx_pt)
    {
      flux_pt_coord << shape_func.flux_line().local_coordinates() ( shape_func.flux_line().points()[0][0][flx_pt] );
      sol_to_flux_pts.row(flx_pt) = shape_func.line().value(flux_pt_coord);
    }

    RealMatrix flx_to_grad_in_sol_pts(nb_sol_pts, nb_flx_pts);
    for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
    {
      sol_pt_coord << shape_func.line().local_coordinates() ( shape_func.line().points()[0][0][sol_pt] );
      flx_to_grad_in_sol_pts.row(sol_pt) = shape_func.flux_line().gradient(sol_pt_coord);
    }

    RealMatrix flx_to_sol_pts(nb_sol_pts, nb_flx_pts);
    for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
    {
      sol_pt_coord << shape_func.line().local_coordinates() ( shape_func.line().points()[0][0][sol_pt] );
      flx_to_sol_pts.row(sol_pt) = shape_func.flux_line().value(sol_pt_coord);
    }

    RealMatrix solution_in_sol_pts(nb_sol_pts, nb_vars);
    RealMatrix solution_in_other_sol_pts(nb_sol_pts, nb_vars);
    RealVector solution_in_other_flx_pt(nb_vars);
    RealRowVector other_solution_sf_values(nb_sol_pts);
    RealVector other_flx_pt_coord(1);
    RealVector riemann_flux(nb_vars);


    Component::Ptr dummy_component;
    Uint dummy_idx;

    Connectivity& c2f = elements.get_child("face_connectivity").as_type<Connectivity>();

    /// element loop
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Connectivity::ConstRow field_idx_proxy = space.indexes_for_element(elem);
      std::vector<Uint> field_idx(field_idx_proxy.size());
      for (Uint i=0; i<field_idx.size(); ++i)
        field_idx[i] = field_idx_proxy[i];

      elements.put_coordinates(geometry_nodes, elem);

      ElementType& geometry = elements.element_type();
      const Uint nb_faces = geometry.nb_faces();
      std::vector<Entities::Ptr> connected_faces(nb_faces);
      std::vector<Uint> connected_face_idx(nb_faces);

      std::vector<Entities::Ptr> connected_cells(nb_faces);
      std::vector<Uint> connected_cell_idx(nb_faces);
      std::vector<bool> connected_cell_is_bdry(nb_faces);

      for (Uint f=0; f<nb_faces; ++f)
      {
        boost::tie(dummy_component,dummy_idx) = c2f.lookup().location( c2f[elem][f] );
        connected_faces[f] = dummy_component->as_ptr<Entities>();
        connected_face_idx[f] = dummy_idx;

        if (CellFaces::Ptr inner_face = connected_faces[f]->as_ptr<CellFaces>())
        {
          FaceCellConnectivity& f2c = inner_face->cell_connectivity();
          for (Uint c=0; c<2; ++c)
          {
            boost::tie(dummy_component,dummy_idx) = f2c.lookup().location( f2c.connectivity()[connected_face_idx[f]][c] );
            if (dummy_component.get() != &elements || dummy_idx != elem)  // this not the current cell
            {
              connected_cells[f] = dummy_component->as_ptr<Entities>();
              connected_cell_idx[f] = dummy_idx;
              connected_cell_is_bdry[f] = false;
              break;
            }
          }
        }
        else
        {
          connected_cell_is_bdry[f] = true;
        }
      }


      /// orientation loop
      for (Uint orientation=0; orientation<shape_func.dimensionality(); ++orientation)
      {

        /// line loop
        for (Uint line=0; line<nb_lines; ++line)
        {
          /// 1) compute solution in solution points in this line
          for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
          {
            const Uint pt_idx = field_idx[ shape_func.points()[orientation][line][sol_pt] ];
            for (Uint var=0; var<nb_vars; ++var)
              solution_in_sol_pts(sol_pt,var) = solution_field[pt_idx][var];
          }

          /// 2) compute solution in flux points in this line
          solution_in_flx_pts = sol_to_flux_pts * solution_in_sol_pts;
          /// 3) compute flux in flux points in this line
          for (Uint flx_pt=0; flx_pt<nb_flx_pts; flx_pt++)
          {
            // Compute plane-jacobian-normal
            face_pt_local_coord.setZero();
            face_pt_local_coord[orientation] = flux_pt_coord[0];
            geometry.compute_plane_jacobian_normal(face_pt_local_coord,geometry_nodes,(CoordRef)orientation,plane_jacobian_normal);
            Real plane_jacob_det = plane_jacobian_normal.norm();
            unit_normal = plane_jacobian_normal/plane_jacob_det;

            if (flx_pt == 0 || flx_pt == nb_flx_pts-1)
            {
              Uint side = flx_pt==0 ? 0 : 1;
              Uint face_nb = shape_func.face_nb()[orientation][side];

              if (connected_cell_is_bdry[face_nb]) // Boundary --> should become boundary condition
              {
                // compute physical properties in flux points
                solution_vars.compute_properties(geometry_nodes.row(0),solution_in_flx_pts.row(flx_pt),dummy_grads, *dummy_props);
                // compute flux in flux points
                solution_vars.flux(*dummy_props,dummy_flux);
                flux_in_flx_pts.row(flx_pt) = dummy_flux * plane_jacobian_normal;
                // compute wave speeds in flux points
                solution_vars.flux_jacobian_eigen_values(*dummy_props,unit_normal, dummy_ev);
                wave_speed_in_flx_pts[flx_pt] = dummy_ev.cwiseAbs().maxCoeff() * plane_jacob_det / 2.;
              }
              else // Cell interface --> solve Riemann problem
              {
                Uint other_flx_pt = flx_pt==0 ? nb_flx_pts-1 : 0 ;
                Connectivity::ConstRow other_field_idx = space.indexes_for_element( connected_cell_idx[face_nb] );
                for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
                {
                  const Uint pt_idx = other_field_idx[ shape_func.points()[orientation][line][sol_pt] ];
                  for (Uint var=0; var<nb_vars; ++var)
                    solution_in_other_sol_pts(sol_pt,var) = solution_field[pt_idx][var];
                }

                other_flx_pt_coord << shape_func.flux_line().local_coordinates() ( shape_func.flux_line().points()[0][0][other_flx_pt] );
                shape_func.line().compute_value( other_flx_pt_coord , other_solution_sf_values );
                solution_in_other_flx_pt = other_solution_sf_values * solution_in_other_sol_pts;

                if (side == 0)
                  m_riemann_solver->compute_interface_flux_and_wavespeeds( solution_in_other_flx_pt , solution_in_flx_pts.row(flx_pt).transpose(), unit_normal, riemann_flux, dummy_ev);
                else
                  m_riemann_solver->compute_interface_flux_and_wavespeeds( solution_in_flx_pts.row(flx_pt).transpose(), solution_in_other_flx_pt, unit_normal, riemann_flux, dummy_ev);
                flux_in_flx_pts.row(flx_pt) = riemann_flux * plane_jacob_det;
                wave_speed_in_flx_pts[flx_pt] = dummy_ev.cwiseAbs().maxCoeff() * plane_jacob_det / 2.; // divided by 2 because reference length = 2
              }
            }
            else
            {
              // compute physical properties in flux points
              solution_vars.compute_properties(dummy_coords,solution_in_flx_pts.row(flx_pt),dummy_grads, *dummy_props);
              // compute flux in flux points
              solution_vars.flux(*dummy_props,dummy_flux);
              flux_in_flx_pts.row(flx_pt) = dummy_flux * plane_jacobian_normal;
              // compute wave speeds in flux points
              solution_vars.flux_jacobian_eigen_values(*dummy_props,unit_normal, dummy_ev);
              wave_speed_in_flx_pts[flx_pt] = dummy_ev.cwiseAbs().maxCoeff() * plane_jacob_det / 2.;

            }
          }

          /// 4) compute derivative of flux to orientation in solution points
          flux_grad_in_sol_pts = flx_to_grad_in_sol_pts * flux_in_flx_pts;
          wave_speed_in_sol_pts = flx_to_sol_pts * wave_speed_in_flx_pts;

//          for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
//          {
//            // compute flux_sf_values and flux_sf_gradient for this solution-point
//            sol_pt_coord << shape_func.line().local_coordinates() ( shape_func.line().points()[0][0][sol_pt] );
//            shape_func.flux_line().compute_gradient( sol_pt_coord , flux_sf_gradient );
//            shape_func.flux_line().compute_value( sol_pt_coord , flux_sf_value );


////            /// IMPORTANT!!! THIS IS A Term
////            // Only take contribution from inner flux points
////            // interval [ 1->nb_flx_pts-1 ] instead of [ 0->nb_flx_pts ]
////            // Face flux points have to be dealt with in a FaceTerm
////            for (Uint flx_pt=0; flx_pt<nb_flx_pts; ++flx_pt)
////            {
////              for (Uint var=0; var<nb_vars; ++var)
////                flux_grad_in_sol_pts(sol_pt,var) += flux_sf_gradient(flx_pt) * flux_in_flx_pts(flx_pt,var);

////              wave_speed_in_sol_pts[sol_pt] += flux_sf_value(flx_pt) * wave_speed_in_flx_pts[flx_pt];
////            }
//          }

          /// 5) add contribution to the residual
          for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
          {
            const Uint pt_idx = field_idx[ shape_func.points()[orientation][line][sol_pt] ];
            for (Uint var=0; var<residual_field.row_size(); ++var)
            {
              residual_field[pt_idx][var] -= flux_grad_in_sol_pts(sol_pt,var) / jacob_det_field[pt_idx][0];
            }
            wave_speed_field[pt_idx][0] += wave_speed_in_sol_pts[sol_pt] / jacob_det_field[pt_idx][0];
          }
        }
        // end line loop
      }
      // end orientation loop
    }
    // end element loop
  }
  // end Cells loop
}

//////////////////////////////////////////////////////////////////////////////

void Convection::compute_cell_interior_flux_points_contribution()
{
  Field& residual_field = residual();
  Field& solution_field = solution();
  Field& wave_speed_field = wave_speed();
  Field& jacob_det_field = jacob_det();

  Variables& solution_vars = *m_solution_vars.lock();
  const Uint nb_vars = solution_field.row_size();

  std::auto_ptr<physics::Properties> phys_props = physical_model().create_properties();
  RealMatrix phys_flux( physical_model().neqs(), physical_model().ndim() );
  RealVector phys_coords( physical_model().ndim() );
  RealMatrix dummy_grads( physical_model().neqs(), physical_model().ndim() );
  RealVector phys_ev(nb_vars);

  /// Cells loop
  boost_foreach(Region::Ptr region, m_loop_regions)
  boost_foreach(Cells& elements, find_components_recursively<Cells>(*region))
  if( field_group().elements_lookup().contains(elements))
  {
    const Space& space = field_group().space(elements);
    const SFDM::ShapeFunction& shape_func = space.shape_function().as_type<SFDM::ShapeFunction>();

    const Uint nb_sol_pts = shape_func.line().nb_nodes();
    const Uint nb_flx_pts = shape_func.flux_line().nb_nodes();
    const Uint nb_lines = shape_func.dimensionality() == 1 ? 1 : nb_sol_pts;

    // In the case of P0 elements with 2 flx_pts, there are no interior flux points,
    // and thus no contribution can be added.
    if (nb_flx_pts == 2) return;

    // Allocations before loop over cells
    RealMatrix geometry_nodes;
    elements.allocate_coordinates(geometry_nodes);
    ElementType& geometry = elements.element_type();
    RealVector unit_normal(geometry.dimension());
    RealVector plane_jacobian_normal(geometry.dimension());
    RealVector local_coord(geometry.dimensionality());
    RealVector flux_pt_line_coord(1);
    RealVector sol_pt_line_coord(1);

    RealMatrix solution_in_flx_pts(nb_flx_pts,nb_vars);
    RealVector wave_speed_in_flx_pts(nb_flx_pts);
    RealMatrix flux_in_flx_pts(nb_flx_pts,nb_vars);

    RealMatrix solution_in_sol_pts(nb_sol_pts, nb_vars);
    RealMatrix flux_grad_in_sol_pts(nb_sol_pts, nb_vars);
    RealVector wave_speed_in_sol_pts(nb_sol_pts);

    // Interpolation matrix from values in solution points to values in flux points
    RealMatrix sol_to_flux_pts(nb_flx_pts, nb_sol_pts);
    for (Uint flx_pt=0; flx_pt<nb_flx_pts; ++flx_pt)
    {
      flux_pt_line_coord << shape_func.flux_line().local_coordinates() ( shape_func.flux_line().points()[0][0][flx_pt] );
      sol_to_flux_pts.row(flx_pt) = shape_func.line().value(flux_pt_line_coord);
    }

    // Interpolation matrix from values in flux points to gradient-values in solution points
    RealMatrix flx_to_grad_in_sol_pts(nb_sol_pts, nb_flx_pts);

    // Interpolation matrix from values in flux points to values in solution points
    RealMatrix flx_to_sol_pts(nb_sol_pts, nb_flx_pts);

    // Computation of interpolation matrices
    for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
    {
      sol_pt_line_coord << shape_func.line().local_coordinates() ( shape_func.line().points()[0][0][sol_pt] );
      flx_to_grad_in_sol_pts.row(sol_pt) = shape_func.flux_line().gradient(sol_pt_line_coord);
      flx_to_sol_pts.row(sol_pt)         = shape_func.flux_line().value(sol_pt_line_coord);
    }

    /// element loop
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
//      if (elements.is_ghost(elem))
//        continue;

      Connectivity::ConstRow field_idx = space.indexes_for_element(elem);

      elements.put_coordinates(geometry_nodes, elem);

      phys_coords = geometry_nodes.row(0);

      ElementType& geometry = elements.element_type();

      /// orientation loop
      for (Uint orientation=0; orientation<shape_func.dimensionality(); ++orientation)
      {

        /// line loop
        for (Uint line=0; line<nb_lines; ++line)
        {

          /// 1) compute solution in solution points in this line
          for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
          {

            cf3_assert(shape_func.points()[orientation][line][sol_pt] < field_idx.size());
            const Uint pt_idx = field_idx[ shape_func.points()[orientation][line][sol_pt] ];
            cf3_assert(pt_idx < solution_field.size());
            for (Uint var=0; var<nb_vars; ++var)
              solution_in_sol_pts(sol_pt,var) = solution_field[pt_idx][var];
          }

          /// 2) compute solution in interior flux points in this line
          for (Uint flx_pt=1; flx_pt<nb_flx_pts-1; flx_pt++) {
            for (Uint var=0; var<nb_vars; ++var) {
              solution_in_flx_pts(flx_pt,var) = 0.;
              for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt) {
                solution_in_flx_pts(flx_pt,var) += sol_to_flux_pts(flx_pt,sol_pt) * solution_in_sol_pts(sol_pt,var);
              }
            }
          }

          /// 3) compute flux and wave_speed in interior flux points in this line
          for (Uint flx_pt=1; flx_pt<nb_flx_pts-1; flx_pt++)
          {
            // Compute plane-jacobian-normal
            local_coord.setZero();
            local_coord[orientation] = flux_pt_line_coord[0];
            geometry.compute_plane_jacobian_normal(local_coord,geometry_nodes,(CoordRef)orientation,plane_jacobian_normal);
            Real plane_jacob_det = plane_jacobian_normal.norm();
            unit_normal = plane_jacobian_normal/plane_jacob_det;

            // compute physical properties in flux points
            solution_vars.compute_properties(phys_coords,solution_in_flx_pts.row(flx_pt),dummy_grads, *phys_props);

            // compute flux in flux points
            solution_vars.flux(*phys_props,phys_flux);
            flux_in_flx_pts.row(flx_pt) = phys_flux * plane_jacobian_normal;

            // compute wave speeds in flux points
            solution_vars.flux_jacobian_eigen_values(*phys_props,unit_normal, phys_ev);
            wave_speed_in_flx_pts(flx_pt) = phys_ev.cwiseAbs().maxCoeff() * plane_jacob_det / 2.;
          }

          /// 4) compute derivative of flux to orientation in solution points
          ///    note that only contribution of inner flux points is added.
          ///    A separate loop to compute contributions of face-flux points is needed
          for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
          {
            for (Uint var=0; var<nb_vars; ++var)
              flux_grad_in_sol_pts(sol_pt,var) = 0.;
            wave_speed_in_sol_pts(sol_pt) = 0.;
            for (Uint flx_pt=1; flx_pt<nb_flx_pts-1; ++flx_pt)
            {
              for (Uint var=0; var<nb_vars; ++var)
                flux_grad_in_sol_pts(sol_pt,var) += flx_to_grad_in_sol_pts(sol_pt,flx_pt) * flux_in_flx_pts(flx_pt,var);
              wave_speed_in_sol_pts(sol_pt) += flx_to_sol_pts(sol_pt,flx_pt) * wave_speed_in_flx_pts(flx_pt);
            }
          }

          /// 5) add contribution to the residual
          for (Uint sol_pt=0; sol_pt<nb_sol_pts; ++sol_pt)
          {
            cf3_assert(shape_func.points()[orientation][line][sol_pt]<field_idx.size());
            const Uint pt_idx = field_idx[ shape_func.points()[orientation][line][sol_pt] ];
            cf3_assert(pt_idx<residual_field.size());
            for (Uint var=0; var<residual_field.row_size(); ++var)
            {
              residual_field[pt_idx][var] -= flux_grad_in_sol_pts(sol_pt,var) / jacob_det_field[pt_idx][0];
            }
//            std::cout << "wave_speed["<<pt_idx<<"]+="<<wave_speed_in_sol_pts[sol_pt] / jacob_det_field[pt_idx][0]<<std::endl;
            wave_speed_field[pt_idx][0] += wave_speed_in_sol_pts[sol_pt] / jacob_det_field[pt_idx][0];
          }
        }
        // end line loop
      }
      // end orientation loop
    }
    // end element loop
  }
  // end Cells loop
}

void Convection::compute_inner_face_flux_points_contribution()
{
  /// @todo Profile and improve efficiency
  Field& residual_field = residual();
  Field& solution_field = solution();
  Field& wave_speed_field = wave_speed();
  Field& jacob_det_field = jacob_det();

  Variables& solution_vars = *m_solution_vars.lock();
  const Uint nb_vars = solution_field.row_size();

  std::auto_ptr<physics::Properties> phys_props = physical_model().create_properties();
  RealMatrix phys_flux( physical_model().neqs(), physical_model().ndim() );
  RealVector phys_coords( physical_model().ndim() );
  RealMatrix dummy_grads( physical_model().neqs(), physical_model().ndim() );
  RealVector phys_ev(nb_vars);

  Component::Ptr dummy_component;
  Uint dummy_idx;

  std::vector<bool> ghost(2);
  std::vector<Cells*> cells(2);
  std::vector<Space*> spaces(2);
  std::vector<SFDM::ShapeFunction const*> sf(2);
  std::vector<Uint> cell_idx(2);
  std::vector<Uint> nb_sol_pts(2);
  std::vector<Uint> nb_flx_pts(2);
  std::vector<Uint> flx_pt(2);
  std::vector<RealVector> flx_pt_line_coord(2);
  std::vector<RealRowVector> solution_at_face(2);


  /// inner-faces loop
  boost_foreach(Region::Ptr region, m_loop_regions)
  boost_foreach(Entities& faces, find_components_recursively_with_tag<Entities>(*region,mesh::Tags::inner_faces()))
  {
    FaceCellConnectivity& cell_connectivity = faces.get_child("cell_connectivity").as_type<FaceCellConnectivity>();

    /// face loop
    for (Uint face=0; face<faces.size(); ++face)
    {
//      std::cout << "f="<<face<<" "<<std::flush;
      Connectivity::ConstRow connected_cells = cell_connectivity.connectivity()[face];
      Connectivity::ConstRow face_nb = cell_connectivity.face_number()[face];

      // Set data for left and right cells
      for (Uint side=0; side<2; ++side)
      {
        boost::tie(dummy_component,dummy_idx) = cell_connectivity.lookup().location(connected_cells[side]);
        cells[side] = &dummy_component->as_type<Cells>();
        cell_idx[side] = dummy_idx;
        spaces[side] = &field_group().space(*cells[side]);
        sf[side] = &spaces[side]->shape_function().as_type<SFDM::ShapeFunction>();
        nb_sol_pts[side] = sf[side]->line().nb_nodes();
        nb_flx_pts[side] = sf[side]->flux_line().nb_nodes();
        flx_pt_line_coord[side].resize(1);
        ghost[side] = cells[side]->is_ghost(cell_idx[side]);
        Connectivity::ConstRow face_nodes = faces.get_nodes(face);
        Connectivity::ConstRow cell_nodes = cells[side]->get_nodes(cell_idx[side]);
        Uint nb_match = 0;
        boost_foreach(const Uint face_node, face_nodes)
        {
          boost_foreach(const Uint cell_node, cell_nodes)
            if (face_node == cell_node)
              ++nb_match;
        }
      }

//      if (ghost[LEFT] && ghost[RIGHT])
//        continue;//throw InvalidStructure(FromHere(),"this is a ghost face!!");

      std::vector<Real> coords(2);

      const Real direction = sf[LEFT]->face_direction(face_nb[LEFT]);

      const Uint orientation = sf[LEFT]->face_orientation(face_nb[LEFT]);

      Uint nb_face_pts = spaces[LEFT]->shape_function().dimensionality() == 1? 1 : spaces[LEFT]->shape_function().order()+1;

      for (Uint line=0; line<nb_face_pts; ++line)
      {

        /// 1) Compute solution at left and right side
        for (Uint side=0; side<2; ++side)
        {
          Connectivity::ConstRow field_idx = spaces[side]->indexes_for_element(cell_idx[side]);

          RealMatrix solution_in_sol_pts(nb_sol_pts[side],nb_vars);
          for (Uint sol_pt=0; sol_pt<nb_sol_pts[side]; ++sol_pt)
          {
            Uint pt_idx = field_idx[ sf[side]->points()[orientation][line][sol_pt] ];
            for (Uint var=0; var<nb_vars; ++var)
              solution_in_sol_pts(sol_pt,var) = solution_field[pt_idx][var];
          }

          flx_pt[side] = sf[side]->face_side(face_nb[side]) == 0 ? 0 : nb_flx_pts[side]-1;
          flx_pt_line_coord[side] << sf[side]->flux_line().local_coordinates()(flx_pt[side],0);

          RealRowVector sf_values(nb_sol_pts[side]);
          sf[side]->line().compute_value(flx_pt_line_coord[side],sf_values);

          solution_at_face[side] = sf_values * solution_in_sol_pts;
        }
        // end side loop

        /// 2) Solve Riemann problem using left and right solution
        RealMatrix left_nodes;
        cells[LEFT]->allocate_coordinates(left_nodes);
        cells[LEFT]->put_coordinates(left_nodes, cell_idx[LEFT]);
        ElementType& left_etype = cells[LEFT]->element_type();

        RealVector face_pt_local_coord(left_etype.dimensionality());
        face_pt_local_coord.setZero();
        face_pt_local_coord[orientation] = flx_pt_line_coord[LEFT][0];
        RealVector plane_jacobian_normal(left_etype.dimension());
        left_etype.compute_plane_jacobian_normal(face_pt_local_coord,left_nodes,(CoordRef)orientation,plane_jacobian_normal);
        Real plane_jacobian_det = plane_jacobian_normal.norm();
        RealVector unit_normal = direction * plane_jacobian_normal / plane_jacobian_det;
        RealVector flux_in_face_pt(nb_vars);
        RealVector dummy_ws(nb_vars);
        m_riemann_solver->compute_interface_flux_and_wavespeeds(solution_at_face[LEFT],solution_at_face[RIGHT],unit_normal,flux_in_face_pt,dummy_ws);
        flux_in_face_pt = flux_in_face_pt * plane_jacobian_det;
        Real wave_speed_in_face_pt = dummy_ws.cwiseAbs().maxCoeff() * plane_jacobian_det / 2.;

        /// 3) Add contribution of face-flux to the residual of cells on both sides
        for (Uint side=0; side<2; ++side)
        {

//          if (ghost[side])
//            continue;

          Connectivity::ConstRow field_idx = spaces[side]->indexes_for_element(cell_idx[side]);

          RealMatrix flux_grad_in_sol_pts(nb_sol_pts[side], nb_vars);
          flux_grad_in_sol_pts.setZero();
          RealVector wave_speed_in_sol_pts(nb_sol_pts[side]);
          wave_speed_in_sol_pts.setZero();

          RealMatrix flux_sf_gradient(1,nb_flx_pts[side]);
          RealRowVector flux_sf_value(nb_flx_pts[side]);
          for (Uint sol_pt=0; sol_pt<nb_sol_pts[side]; ++sol_pt)
          {
            // compute flux_sf_gradient
            RealVector sol_pt_coord(1);
            sol_pt_coord << sf[side]->line().local_coordinates() ( sf[side]->line().points()[0][0][sol_pt] , 0);
            sf[side]->flux_line().compute_gradient( sol_pt_coord , flux_sf_gradient );
            sf[side]->flux_line().compute_value   ( sol_pt_coord , flux_sf_value    );

            for (Uint var=0; var<nb_vars; ++var)
            {
              flux_grad_in_sol_pts(sol_pt,var) += flux_sf_gradient(flx_pt[side]) * flux_in_face_pt[var];
            }

            wave_speed_in_sol_pts[sol_pt] += flux_sf_value(flx_pt[side]) * wave_speed_in_face_pt;


//            std::cout << "flux_grad_in_sol_pts["<<cell_idx[side]<<"]["<<orientation<<"]["<<line<<"]["<<sol_pt<<"] = " << flux_grad_in_sol_pts.row(sol_pt) << std::endl;
            const Uint pt_idx = field_idx[ sf[side]->points()[orientation][line][sol_pt] ];

//            std::cout << "face adding wave_speed to cell["<<cell_idx[side]<<"]    " << wave_speed_in_sol_pts(sol_pt) << std::endl;
            for (Uint var=0; var<residual_field.row_size(); ++var)
            {
              residual_field[pt_idx][var] -= direction * flux_grad_in_sol_pts(sol_pt,var) / jacob_det_field[pt_idx][0];
            }
            wave_speed_field[pt_idx][0] += wave_speed_in_sol_pts[sol_pt] / jacob_det_field[pt_idx][0];
          }

        }
        // end side loop
      }
      // end line loop
    }
    // end face loop
  }
  // end inner faces loop


  /// outer-faces loop
  /// These faces should be moved to the boundary condition
  boost_foreach(Region::Ptr region, m_loop_regions)
  boost_foreach(Entities& faces, find_components_recursively<Entities>(*region))
  {
    if (faces.as_ptr<Faces>())
    {
      FaceCellConnectivity& cell_connectivity = faces.get_child("cell_connectivity").as_type<FaceCellConnectivity>();

      /// face loop
      for (Uint face=0; face<faces.size(); ++face)
      {
        Connectivity::ConstRow connected_cells = cell_connectivity.connectivity()[face];
        Connectivity::ConstRow face_nb = cell_connectivity.face_number()[face];
        for (Uint side=0; side<1; ++side)
        {
          boost::tie(dummy_component,dummy_idx) = cell_connectivity.lookup().location(connected_cells[side]);
          cells[side] = &dummy_component->as_type<Cells>();
          cell_idx[side] = dummy_idx;
          spaces[side] = &field_group().space(*cells[side]);
          sf[side] = &spaces[side]->shape_function().as_type<SFDM::ShapeFunction>();
          nb_sol_pts[side] = sf[side]->line().nb_nodes();
          nb_flx_pts[side] = sf[side]->flux_line().nb_nodes();
          flx_pt_line_coord[side].resize(1);

          Connectivity::ConstRow face_nodes = faces.get_nodes(face);
          Connectivity::ConstRow cell_nodes = cells[side]->get_nodes(cell_idx[side]);
          Uint nb_match = 0;
          boost_foreach(const Uint face_node, face_nodes)
          {
            boost_foreach(const Uint cell_node, cell_nodes)
              if (face_node == cell_node)
                ++nb_match;
          }
        }

        std::vector<RealRowVector> solution_at_face(2);
        Uint orientation = sf[0]->face_orientation(face_nb[0]);
        const Real direction = sf[LEFT]->face_direction(face_nb[LEFT]);


        Uint nb_face_pts = spaces[0]->shape_function().dimensionality() == 1? 1 : spaces[0]->shape_function().order()+1;

        for (Uint line=0; line<nb_face_pts; ++line)
        {
          for (Uint side=0; side<1; ++side)
          {
            Connectivity::ConstRow field_idx = spaces[side]->indexes_for_element(cell_idx[side]);

            RealMatrix solution_in_sol_pts(nb_sol_pts[side],nb_vars);
            for (Uint sol_pt=0; sol_pt<nb_sol_pts[side]; ++sol_pt)
            {
              Uint pt_idx = field_idx[ sf[side]->points()[orientation][line][sol_pt] ];
              for (Uint var=0; var<nb_vars; ++var)
                solution_in_sol_pts(sol_pt,var) = solution_field[pt_idx][var];
            }

            flx_pt[side] = sf[side]->face_side(face_nb[side]) == 0 ? 0 : nb_flx_pts[side]-1;
            flx_pt_line_coord[side] << sf[side]->flux_line().local_coordinates()(flx_pt[side],0);

            RealRowVector sf_values(nb_sol_pts[side]);
            sf[side]->line().compute_value(flx_pt_line_coord[side],sf_values);

            solution_at_face[side] = sf_values * solution_in_sol_pts;
          }


          RealMatrix left_nodes;
          cells[0]->allocate_coordinates(left_nodes);
          cells[0]->put_coordinates(left_nodes, cell_idx[0]);
          ElementType& left_etype = cells[0]->element_type();

          RealVector face_pt_local_coord(left_etype.dimensionality());
          face_pt_local_coord.setZero();
          face_pt_local_coord[orientation] = flx_pt_line_coord[LEFT][0];
          RealVector plane_jacobian_normal(left_etype.dimension());
          left_etype.compute_plane_jacobian_normal(face_pt_local_coord,left_nodes,(CoordRef)orientation,plane_jacobian_normal);
          Real plane_jacobian_det = plane_jacobian_normal.norm();
          RealVector unit_normal = plane_jacobian_normal / plane_jacobian_det;

          //std::cout << "compute_flux( " << solution_at_face[0] << "   ,   " << solution_at_face[1] << "    ,    " << plane_jacobian_normal.transpose() << "  ) " << std::endl;
          RealVector flux_in_face_pt(nb_vars);
          solution_vars.compute_properties(phys_coords,solution_at_face[0],dummy_grads,*phys_props);
          solution_vars.flux(*phys_props,phys_flux);
          solution_vars.flux_jacobian_eigen_values(*phys_props,unit_normal,phys_ev);
          Real wave_speeds_in_face_pt = phys_ev.cwiseAbs().maxCoeff() * plane_jacobian_det / 2.;

          flux_in_face_pt = phys_flux * plane_jacobian_normal;

          for (Uint side=0; side<1; ++side)
          {

            Connectivity::ConstRow field_idx = spaces[side]->indexes_for_element(cell_idx[side]);


            /// 4) compute derivative of flux to orientation in solution points
            RealMatrix flux_grad_in_sol_pts(nb_sol_pts[side], nb_vars);
            flux_grad_in_sol_pts.setZero();
            RealVector wave_speed_in_sol_pts(nb_sol_pts[side]);
            wave_speed_in_sol_pts.setZero();

            RealMatrix flux_sf_gradient(1,nb_flx_pts[side]);
            RealRowVector flux_sf_value(nb_flx_pts[side]);
            for (Uint sol_pt=0; sol_pt<nb_sol_pts[side]; ++sol_pt)
            {
              // compute flux_sf_gradient
              RealVector sol_pt_coord(1);
              sol_pt_coord << sf[side]->line().local_coordinates() ( sf[side]->line().points()[0][0][sol_pt] , 0);
              sf[side]->flux_line().compute_gradient( sol_pt_coord , flux_sf_gradient );
              sf[side]->flux_line().compute_value   ( sol_pt_coord , flux_sf_value    );

              for (Uint var=0; var<nb_vars; ++var)
              {
                flux_grad_in_sol_pts(sol_pt,var) += flux_sf_gradient(flx_pt[side]) * flux_in_face_pt[var];
              }
              wave_speed_in_sol_pts[sol_pt] += flux_sf_value(flx_pt[side]) * wave_speeds_in_face_pt;

              //std::cout << "flux_grad_in_sol_pts["<<cell_idx[side]<<"]["<<orientation<<"]["<<line<<"]["<<sol_pt<<"] = " << flux_grad_in_sol_pts.row(sol_pt) << std::endl;
              const Uint pt_idx = field_idx[ sf[side]->points()[orientation][line][sol_pt] ];

              for (Uint var=0; var<residual_field.row_size(); ++var)
              {
                residual_field[pt_idx][var] -= flux_grad_in_sol_pts(sol_pt,var) / jacob_det_field[pt_idx][0];
              }
              wave_speed_field[pt_idx][0] += wave_speed_in_sol_pts[sol_pt] / jacob_det_field[pt_idx][0];
            }

          }
        }
      }
    }
    // end outer faces loop
  }
}

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
