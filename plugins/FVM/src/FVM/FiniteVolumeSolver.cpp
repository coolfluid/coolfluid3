// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>
#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"

#include "FVM/FiniteVolumeSolver.hpp"
#include "FVM/ComputeUpdateCoefficient.hpp"
#include "FVM/UpdateSolution.hpp"
#include "FVM/ComputeFlux.hpp"
#include "FVM/OutputIterationInfo.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/Actions/CInitFieldConstant.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/Actions/CForAllFaces.hpp"
#include "Solver/Actions/CLoop.hpp"
#include "Solver/Actions/CIterate.hpp"
#include "Solver/Actions/CCriterionTime.hpp"
#include "Solver/Actions/CAdvanceTime.hpp"


#include "Math/MathConsts.hpp"

#include "Mesh/Actions/CInitFieldConstant.hpp"
#include "Mesh/Actions/CBuildFaceNormals.hpp"

namespace CF {
namespace FVM {

using namespace boost::assign;
using namespace Common;
using namespace Mesh;
using namespace Mesh::Actions;
using namespace Solver;
using namespace Solver::Actions;

Common::ComponentBuilder < FiniteVolumeSolver, CSolver, LibFVM > FiniteVolumeSolver_Builder;

////////////////////////////////////////////////////////////////////////////////

FiniteVolumeSolver::FiniteVolumeSolver ( const std::string& name  ) : CSolver ( name )
{
  properties()["brief"] = std::string("Forward Euler Time Stepper with Finite Volume Method");
  std::string description =
    "Forward Euler:\n"
    " U[n+1] = U[n] + dt/dx * R \n"
    " 1) delegate computation of the residual and wave_speed to the discretization method\n"
    " 2) compute the update coefficient = dt/dx = CFL/wave_speed"
    " 3) solution = update_coeff * residual\n"
    "\n"
    "Finite Volume Method:\n"
    "Discretize the PDE's using the Cell Centered Finite Volume Method\n"
    "This method is expected to fill the residual field, and the wave_speed field.\n"
    " - The residual for cell[i] being F[i+1/2] - F[i-1/2],\n"
    "   F[i+1/2] is calculated using an approximate Riemann solver on the face\n"
    "   between cell[i] and cell[i+1]\n"
    " - The wave_speed being the wave speed \"a\" in the Courant number defined as:\n"
    "        CFL = a * dt / V ,\n"
    "   with V the volume of a cell (notice not length) and dt the timestep to take.";
    
  properties()["description"] = description;

  m_properties["Domain"].as_option().attach_trigger ( boost::bind ( &FiniteVolumeSolver::trigger_Domain,   this ) );

  this->regist_signal ( "solve" , "Solve", "Solve" )->connect ( boost::bind ( &FiniteVolumeSolver::solve, this ) );

  m_solution = create_static_component<CLink>("solution");
  m_residual = create_static_component<CLink>("residual");
  m_wave_speed = create_static_component<CLink>("wave_speed");
  m_update_coeff = create_static_component<CLink>("update_coeff");


  m_iterate = create_static_component<CIterate>("iterate");

  // create apply boundary conditions action
  m_apply_bcs = m_iterate->create_static_component<CAction>("1_apply_boundary_conditions");
  m_apply_bcs->mark_basic();
  
  // create compute rhs action
  m_compute_rhs = m_iterate->create_static_component<CAction>("2_compute_rhs");
  m_compute_rhs->mark_basic();
  
  // set the compute rhs action
  m_compute_rhs->create_static_component<CInitFieldConstant>("2.1_init_residual")
    ->configure_property("Constant",0.)
    .mark_basic()
    .property("Field").as_option().add_tag("residual");
  
  m_compute_rhs->create_static_component<CInitFieldConstant>("2.2_init_wave_speed")
    ->configure_property("Constant",Math::MathConsts::eps())
    .mark_basic()
    .property("Field").as_option().add_tag("wave_speed");
  
  m_compute_rhs->create_static_component<CForAllFaces>("2.3_for_all_inner_faces")
    ->mark_basic()
    .create_static_component<ComputeFlux>("add_flux_to_rhs")
      ->mark_basic();
  
  m_compute_update_coefficient = m_iterate->create_static_component<ComputeUpdateCoefficient>("3_compute_update_coeff");
  m_update_solution = m_iterate->create_static_component<UpdateSolution>("4_update_solution");
  m_iterate->create_static_component<CAdvanceTime>("5_advance_time");
  m_iterate->create_static_component<OutputIterationInfo>("6_output_info");
  m_iterate->create_static_component<CCriterionTime>("time_stop_criterion");
}

////////////////////////////////////////////////////////////////////////////////

FiniteVolumeSolver::~FiniteVolumeSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::trigger_Domain()
{
  URI domain; property("Domain").put_value(domain);

  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>(*access_component_ptr(domain));
  if (is_null(mesh))
    throw SetupError(FromHere(),"Domain has no mesh");

  m_compute_rhs->get_child_ptr("2.3_for_all_inner_faces")
    ->configure_property("Regions",std::vector<URI>(1,mesh->topology().full_path()));

  if ( is_null(find_component_ptr_with_tag<CField2>(*mesh,"face_normal") ) )
  {
    CBuildFaceNormals::Ptr build_face_normals = create_component<CBuildFaceNormals>("build_face_normals");
    build_face_normals->transform(mesh);
    remove_component(build_face_normals->name());
    configure_option_recursively("face_normal", find_component_with_tag<CField2>(*mesh,"face_normal").full_path());
  }

  if ( is_null(find_component_ptr_with_tag<CField2>(*mesh,"area") ) )
  {
    CField2& area = mesh->create_field2("area","FaceBased");
    area.add_tag("area");
    CLoop::Ptr compute_area = create_component< CForAllFaces >("compute_area");
    compute_area->configure_property("Regions", std::vector<URI>(1,area.topology().full_path()));
    compute_area->create_action("CF.Solver.Actions.CComputeArea");
    configure_option_recursively("area",area.full_path());
    compute_area->execute();
    remove_component(compute_area->name());
  }
  
  // create/initialize a solution if it is not available
  CField2::Ptr solution_ptr = find_component_ptr_with_tag<CField2>(*mesh,"solution");
  if ( is_null(solution_ptr) )
  {
    ///@todo get variable names etc, from Physics
    CFinfo << "++++ Creating cell based solution field with vars rho[1],rhoU[1],rhoE[1]" << CFendl;
    CField2& solution = mesh->create_field2("solution","CellBased","rho[1],rhoU[1],rhoE[1]");
    solution.add_tag("solution"); 
  }


  CField2& solution = find_component_with_tag<CField2>(*mesh,"solution");
  m_solution->link_to(solution.self());    

  Component::Ptr residual_ptr = find_component_ptr_with_tag(*mesh,"residual");
  if ( is_null(residual_ptr) )
  {
    residual_ptr = mesh->create_field2("residual",solution).self();
    residual_ptr->add_tag("residual");
  }
  m_residual->link_to(residual_ptr);

  Component::Ptr wave_speed_ptr = find_component_ptr_with_tag(*mesh,"wave_speed");
  if ( is_null(wave_speed_ptr) )
  {
    wave_speed_ptr = mesh->create_scalar_field("wave_speed",solution).self();
    wave_speed_ptr->add_tag("wave_speed");
  }
  m_wave_speed->link_to(wave_speed_ptr);

  Component::Ptr update_coeff_ptr = find_component_ptr_with_tag(*mesh,"update_coeff");
  if ( is_null(update_coeff_ptr) )
  {
    update_coeff_ptr = mesh->create_scalar_field("update_coeff",solution).self();
    update_coeff_ptr->add_tag("update_coeff");
  }
  m_update_coeff->link_to(update_coeff_ptr);

  configure_option_recursively("solution",solution.full_path());
  configure_option_recursively("wave_speed",wave_speed_ptr->full_path());
  configure_option_recursively("residual",residual_ptr->full_path());
  configure_option_recursively("update_coeff",update_coeff_ptr->full_path());
  
}

//////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::solve()
{
  if ( is_null(m_solution->follow()) )  throw SetupError (FromHere(), "solution is not linked to solution field");
  m_iterate->execute();
}

////////////////////////////////////////////////////////////////////////////////

CAction& FiniteVolumeSolver::create_bc(const std::string& name, const std::vector<CRegion::Ptr>& regions, const std::string& bc_builder_name)
{
  std::vector<URI> regions_uri; regions_uri.reserve(regions.size());
  boost_foreach(CRegion::Ptr region, regions)
    regions_uri.push_back(region->full_path());

  CAction::Ptr for_all_faces = m_apply_bcs->create_component<CForAllFaces>(name);
  for_all_faces->configure_property("Regions",regions_uri);
  CAction& bc = for_all_faces->create_action(bc_builder_name,bc_builder_name);
  configure_option_recursively("solution", m_solution->follow()->full_path());    
  return bc;
}

////////////////////////////////////////////////////////////////////////////////

CAction& FiniteVolumeSolver::create_bc(const std::string& name, const CRegion& region, const std::string& bc_builder_name)
{
  CAction::Ptr for_all_faces = m_apply_bcs->create_component<CForAllFaces>(name);
  for_all_faces->configure_property("Regions",std::vector<URI>(1,region.full_path()));
  CAction& bc = for_all_faces->create_action(bc_builder_name,bc_builder_name);
  configure_option_recursively("solution", m_solution->follow()->full_path());    
  return bc;
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
