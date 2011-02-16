// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/Actions/CBuildFaceNormals.hpp"

#include "Mesh/CField2.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"

#include "Solver/CIterativeSolver.hpp"
#include "Solver/Actions/CForAllFaces.hpp"

#include "FVM/FiniteVolume.hpp"
#include "FVM/ComputeFlux.hpp"

using namespace boost::assign;

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

Common::ComponentBuilder < FiniteVolume, CDiscretization, LibFVM > FiniteVolume_Builder;

////////////////////////////////////////////////////////////////////////////////

FiniteVolume::FiniteVolume ( const std::string& name  ) :
  CDiscretization ( name )
{
  // properties

  properties()["brief"] = std::string("Finite Volume Method");
  std::string description =
    "Discretize the PDE's using the Cell Centered Finite Volume Method\n"
    "This method is expected to fill the residual field, and the advection field.\n"
    " - The residual for cell[i] being F[i+1/2] - F[i-1/2],\n"
    "   F[i+1/2] is calculated using an approximate Riemann solver on the face\n"
    "   between cell[i] and cell[i+1]\n"
    " - The advection being the wavespeed \"a\" in the Courant number defined as:\n"
    "        CFL = a * dt / V ,\n"
    "   with V the volume of a cell (notice not length) and dt the timestep to take.";
  properties()["description"] = description; 

  // create apply boundary conditions action
  m_apply_bcs = create_static_component<CAction>("apply_boundary_conditions");
  m_apply_bcs->mark_basic();
  
  // create compute rhs action
  m_compute_rhs = create_static_component<CAction>("compute_rhs");
  m_compute_rhs->mark_basic();
  
  // set the compute rhs action
  CAction::Ptr for_all_faces = m_compute_rhs->create_static_component<CForAllFaces>("for_all_inner_faces");
  for_all_faces->mark_basic();
  for_all_faces->create_static_component<ComputeFlux>("add_flux_to_rhs")->mark_basic();
  
  properties()["Mesh"].as_option().attach_trigger( boost::bind( &FiniteVolume::on_config_mesh, this));
  
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolume::on_config_mesh()
{
  m_compute_rhs->get_child("for_all_inner_faces")
    ->configure_property("Regions",std::vector<URI>(1,m_mesh.lock()->topology().full_path()));

  if ( is_null(find_component_ptr_with_tag<CField2>(*m_mesh.lock(),"face_normal") ) )
  {
    CBuildFaceNormals::Ptr build_face_normals = create_component<CBuildFaceNormals>("build_face_normals");
    build_face_normals->transform(m_mesh.lock());
    remove_component(build_face_normals->name());
    configure_option_recursively("face_normal", find_component_with_tag<CField2>(*m_mesh.lock(),"face_normal").full_path());    
  }

  if ( is_null(find_component_ptr_with_tag<CField2>(*m_mesh.lock(),"area") ) )
  {
    CField2& area = m_mesh.lock()->create_field2("area","FaceBased");
    area.add_tag("area");
    CLoop::Ptr compute_area = create_component< CForAllFaces >("compute_area");
    compute_area->configure_property("Regions", std::vector<URI>(1,area.topology().full_path()));
    compute_area->create_action("CF.Solver.Actions.CComputeArea");
    configure_option_recursively("area",area.full_path());
    compute_area->execute();
    remove_component(compute_area->name());
  }

}

////////////////////////////////////////////////////////////////////////////////

FiniteVolume::~FiniteVolume()
{
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolume::compute_rhs()
{
  m_apply_bcs->execute();
  m_compute_rhs->execute();
}

////////////////////////////////////////////////////////////////////////////////

CAction& FiniteVolume::create_bc(const std::string& name, const std::vector<CRegion::Ptr>& regions, const std::string& bc_builder_name)
{
  std::vector<URI> regions_uri; regions_uri.reserve(regions.size());
  boost_foreach(CRegion::Ptr region, regions)
    regions_uri.push_back(region->full_path());

  CAction::Ptr for_all_faces = m_apply_bcs->create_component<CForAllFaces>(name);
  for_all_faces->configure_property("Regions",regions_uri);
  return for_all_faces->create_action(bc_builder_name,"bc");
}

////////////////////////////////////////////////////////////////////////////////

CAction& FiniteVolume::create_bc(const std::string& name, const CRegion& region, const std::string& bc_builder_name)
{
  CAction::Ptr for_all_faces = m_apply_bcs->create_component<CForAllFaces>(name);
  for_all_faces->configure_property("Regions",std::vector<URI>(1,region.full_path()));
  return for_all_faces->create_action(bc_builder_name,"bc");
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
