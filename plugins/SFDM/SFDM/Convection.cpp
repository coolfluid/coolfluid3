// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/CellFaces.hpp"
#include "mesh/Field.hpp"
#include "mesh/UnifiedData.hpp"
#include "mesh/FaceCellConnectivity.hpp"

#include "physics/PhysModel.hpp"

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
}

/////////////////////////////////////////////////////////////////////////////

void Convection::allocate_fast_access_data()
{
  // for cell term
  sol_in_flx_pt.resize(physical_model().neqs());
  flx_in_flx_pt.resize(physical_model().neqs());

  // for face term
  face_at_side.resize(2);
  flx_pt_at_side.resize(2);
  sol_at_side.resize(2);
  sol_at_side[LEFT].resize(physical_model().neqs());
  sol_at_side[RIGHT].resize(physical_model().neqs());
}

/////////////////////////////////////////////////////////////////////////////

#ifdef SANDBOX
void Convection::execute()
{
  std::cout << name() << " for " << m_entities->uri() << "["<<m_elem_idx<<"]"<<std::endl;
}

#else
void Convection::execute()
{
  link_fields();
  allocate_fast_access_data();

  // Set residual and wave_speeds to zero
  residual_field() = 0.;
  wave_speed_field() = 0.;

  compute_interior_flx_pts_contribution();
  compute_face_flx_pts_contribution();
  apply_null_bc();
}
#endif
//////////////////////////////////////////////////////////////////////////////

void Convection::compute_interior_flx_pts_contribution()
{
  /// Cells loop
  boost_foreach(Handle< Region > region, m_loop_regions)
  boost_foreach(Cells& elements, find_components_recursively<Cells>(*region))
  if( solution_field().elements_lookup().contains(elements))
  {
    Flyweight fly = create_flyweight(elements);

    // In the case of P0 elements with 1 sol_pt, there are no interior flux points,
    // and thus no contribution can be added.
    if (fly.sf.interior_flx_pts().size()==0) return;

    /// For every element
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      fly.set_element(elem);

      /// For every interior flux point
      boost_foreach(const Uint flx_pt, fly.sf.interior_flx_pts())
      {
        /// 1) compute solution in interior flux point
        fly.reconstruct_solution_in_flx_pt(flx_pt,sol_in_flx_pt);

        /// 2) compute physical flux from solution
        fly.compute_analytical_flux(flx_pt,sol_in_flx_pt,flx_in_flx_pt,wave_speed_in_flx_pt);

        /// 4) add contribution of this flux-point to solution points for the
        ///    gradient of the flux
        fly.add_flx_pt_gradient_contribution_to_residual(flx_pt,flx_in_flx_pt);

        /// 5) add contribution of this flux-point to solution points for the
        ///    interpolation of the wave_speed
        fly.add_flx_pt_contribution_to_wave_speed(flx_pt,wave_speed_in_flx_pt);
      }
      // end flux point loop
    }
    // end element loop
  }
  // end Cells loop
}

//////////////////////////////////////////////////////////////////////////////

void Convection::compute_face_flx_pts_contribution()
{
  /// Faces loop
  std::vector< boost::shared_ptr<Flyweight> > fly(2);

  boost_foreach(Handle< Region > region, m_loop_regions)
  boost_foreach(Entities& faces, find_components_recursively_with_tag<Entities>(*region,mesh::Tags::inner_faces()))
  {
    FaceCellConnectivity& cell_connectivity = *faces.get_child("cell_connectivity")->handle<FaceCellConnectivity>();
    /// For every face
    for (Face2Cell face(cell_connectivity); face.idx<cell_connectivity.size(); ++face.idx)
    {
      fly = create_flyweight(face);
      const Uint nb_face_pts = fly[LEFT]->sf.face_flx_pts(face.face_nb_in_cells()[LEFT]).size();
      cf3_assert(nb_face_pts == fly[RIGHT]->sf.face_flx_pts(face.face_nb_in_cells()[RIGHT]).size());

      /// For every face flux point
      for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
      {
        /// 1) compute solution in flux point on both sides of the face
        for (Uint side=0; side<2; ++side)
        {
          face_at_side[side] = face.face_nb_in_cells()[side];
          if (side==RIGHT)
          {
            if(face_at_side[LEFT]==0 || face_at_side[LEFT]==1)
            {
              if (face_at_side[RIGHT]==0 || face_at_side[RIGHT]==1)
              {
                flx_pt_at_side[side] = fly[side]->sf.face_flx_pts(face_at_side[side])[nb_face_pts-1-face_pt];
              }
              else
              {
                flx_pt_at_side[side] = fly[side]->sf.face_flx_pts(face_at_side[side])[face_pt];
              }
            }
            else if(face_at_side[LEFT]==2 || face_at_side[LEFT]==3)
            {
              if (face_at_side[RIGHT]==2 || face_at_side[RIGHT]==3)
              {
                flx_pt_at_side[side] = fly[side]->sf.face_flx_pts(face_at_side[side])[nb_face_pts-1-face_pt];
              }
              else
              {
                flx_pt_at_side[side] = fly[side]->sf.face_flx_pts(face_at_side[side])[face_pt];
              }
            }
          }
          else
            flx_pt_at_side[side] = fly[side]->sf.face_flx_pts(face_at_side[side])[face_pt];

          fly[side]->reconstruct_solution_in_flx_pt(flx_pt_at_side[side],sol_at_side[side]);

//          RealMatrix cell_coords = fly[side]->entities.get_coordinates(fly[side]->element->idx);
//          flx_pt_coord[side] = fly[side]->geometry.shape_function().value(fly[side]->sf.flx_pts().row(flx_pt_at_side[side]))*cell_coords;
//          if (side == RIGHT)
//            cf3_assert(flx_pt_coord[LEFT]==flx_pt_coord[RIGHT]);
        }

        /// 2) compute numerical flux in this point
        fly[LEFT]->compute_numerical_flux(flx_pt_at_side[LEFT],sol_at_side[LEFT],sol_at_side[RIGHT],
                                         flx_in_flx_pt,wave_speed_in_flx_pt);

        for (Uint side=0; side<2; ++side)
        {
          /// 3) add contribution of this flux-point to solution points for the
          ///    gradient of the flux
          fly[side]->add_flx_pt_gradient_contribution_to_residual(flx_pt_at_side[side],flx_in_flx_pt, (side==0));

          /// 4) add contribution of this flux-point to solution points for the
          ///    interpolation of the wave_speed
          fly[side]->add_flx_pt_contribution_to_wave_speed(flx_pt_at_side[side],wave_speed_in_flx_pt);
        }
      }
      // end face_pt loop
    }
    // end face loop
  }
  // end inner faces loop
}
\
/////////////////////////////////////////////////////////////////////////////

void Convection::apply_null_bc()
{
  /// Faces loop
  boost_foreach(const Handle<Region>& region, m_loop_regions)
  boost_foreach(Entities& faces, find_components_recursively_with_tag<Entities>(*region,mesh::Tags::outer_faces()))
  {
    FaceCellConnectivity& cell_connectivity = *faces.get_child("cell_connectivity")->handle<FaceCellConnectivity>();

    /// For every face
    for (Face2Cell face(cell_connectivity); face.idx<cell_connectivity.size(); ++face.idx)
    {
      Flyweight fly = create_flyweight(face.cells()[LEFT]);
      const Uint nb_face_pts = fly.sf.face_flx_pts(face.face_nb_in_cells()[LEFT]).size();
      const Uint face_nb = face.face_nb_in_cells()[LEFT];

      /// For every face flux point
      boost_foreach(const Uint flx_pt, fly.sf.face_flx_pts(face_nb))
      {
        /// 1) compute solution in face flux point
        fly.reconstruct_solution_in_flx_pt(flx_pt,sol_in_flx_pt);

        /// 2) compute analytical flux in this point
        fly.compute_analytical_flux(flx_pt,sol_in_flx_pt,flx_in_flx_pt,wave_speed_in_flx_pt);

        /// 3) add contribution of this flux-point to solution points for the
        ///    gradient of the flux
        fly.add_flx_pt_gradient_contribution_to_residual(flx_pt,flx_in_flx_pt);

        /// 5) add contribution of this flux-point to solution points for the
        ///    interpolation of the wave_speed
        fly.add_flx_pt_contribution_to_wave_speed(flx_pt,wave_speed_in_flx_pt);
      }
      // end face_pt loop
    }
    // end face loop
  }
  // end outer faces loop
}

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
