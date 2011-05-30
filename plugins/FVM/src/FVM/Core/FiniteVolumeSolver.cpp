// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>
#include <boost/assign/list_of.hpp>

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroupActions.hpp"
 
#include "Common/XML/SignalOptions.hpp"

#include "FVM/Core/FiniteVolumeSolver.hpp"
#include "FVM/Core/ComputeUpdateCoefficient.hpp"
#include "FVM/Core/UpdateSolution.hpp"
#include "FVM/Core/OutputIterationInfo.hpp"
#include "FVM/Core/BC.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/Actions/CInitFieldConstant.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CTime.hpp"

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
namespace Core {

using namespace boost::assign;
using namespace Common;
using namespace Mesh;
using namespace Mesh::Actions;
using namespace Solver;
using namespace Solver::Actions;

Common::ComponentBuilder < FiniteVolumeSolver, CSolver, LibCore > FiniteVolumeSolver_Builder;

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


  // Properties
  property("domain").as_option().attach_trigger ( boost::bind ( &FiniteVolumeSolver::trigger_domain,   this ) );

  properties().add_option(OptionURI::create("physical_model","Physical Model","cpath:../Physics",URI::Scheme::CPATH))
    ->attach_trigger( boost::bind ( &FiniteVolumeSolver::trigger_physical_model, this ) );

  properties().add_option(OptionURI::create("time","Time","Time tracking component","cpath:../Time",URI::Scheme::CPATH))
    ->attach_trigger( boost::bind ( &FiniteVolumeSolver::trigger_time, this ) );

  // Signals
  this->regist_signal ( "create_bc" , "Create Boundary Condition", "Create Boundary Condition" )->signal->connect ( boost::bind ( &FiniteVolumeSolver::signal_create_bc, this , _1) );


  // initializations
  m_solution = create_static_component_ptr<CLink>("solution");
  m_residual = create_static_component_ptr<CLink>("residual");
  m_wave_speed = create_static_component_ptr<CLink>("wave_speed");
  m_update_coeff = create_static_component_ptr<CLink>("update_coeff");


  m_iterate = create_static_component_ptr<CIterate>("iterate");

  // create apply boundary conditions action
  m_apply_bcs = m_iterate->create_static_component_ptr<CGroupActions>("1_apply_boundary_conditions");
  m_apply_bcs->mark_basic();

  // create compute rhs action
  m_compute_rhs = m_iterate->create_static_component_ptr<CGroupActions>("2_compute_rhs");
  m_compute_rhs->mark_basic();

  // set the compute rhs action
  m_compute_rhs->create_static_component_ptr<CInitFieldConstant>("2.1_init_residual")
    ->configure_property("constant",0.)
    .mark_basic()
    .property("field").as_option().add_tag("residual");

  m_compute_rhs->create_static_component_ptr<CInitFieldConstant>("2.2_init_wave_speed")
    ->configure_property("constant",Math::MathConsts::eps())
    .mark_basic()
    .property("field").as_option().add_tag("wave_speed");

  m_compute_rhs->create_static_component_ptr<CForAllFaces>("2.3_for_all_faces")
    ->mark_basic();

  m_compute_update_coefficient = m_iterate->create_static_component_ptr<ComputeUpdateCoefficient>("3_compute_update_coeff");
  m_update_solution = m_iterate->create_static_component_ptr<UpdateSolution>("4_update_solution");
  m_iterate->create_static_component_ptr<CAdvanceTime>("5_advance_time");
  m_iterate->create_static_component_ptr<OutputIterationInfo>("6_output_info");
  m_iterate->create_static_component_ptr<CCriterionTime>("time_stop_criterion");
}

////////////////////////////////////////////////////////////////////////////////

FiniteVolumeSolver::~FiniteVolumeSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::trigger_domain()
{
  if (m_physical_model.expired())
    trigger_physical_model();

  URI domain; property("domain").put_value(domain);
  m_domain = access_component_ptr_checked(domain)->as_ptr_checked<CDomain>();
  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>(*m_domain.lock());
  if (is_null(mesh))
    throw SetupError(FromHere(),"Domain has no mesh");

  m_compute_rhs->get_child_ptr("2.3_for_all_faces")
    ->configure_property("regions",std::vector<URI>(1,mesh->topology().uri()));
  CLoopOperation::Ptr add_flux_to_rhs = build_component_abstract_type<CLoopOperation>("CF.FVM.Core.ComputeFlux","add_flux_to_rhs");
  add_flux_to_rhs->mark_basic();
  m_compute_rhs->get_child("2.3_for_all_faces").add_component(add_flux_to_rhs);

  if ( is_null(find_component_ptr_with_tag<CField>(*mesh,Mesh::Tags::normal()) ) )
  {
    CFinfo << "  Creating field \"face_normal\", facebased" << CFendl;
    CBuildFaceNormals::Ptr build_face_normals = create_component_ptr<CBuildFaceNormals>("build_face_normals");
    build_face_normals->transform(mesh);
    remove_component(build_face_normals->name());
    configure_option_recursively(Mesh::Tags::normal(), find_component_with_tag<CField>(*mesh,Mesh::Tags::normal()).uri());
  }

  if ( is_null(find_component_ptr_with_name<CField>(*mesh,"area") ) )
  {
    CFinfo << "  Creating field \"area\", facebased" << CFendl;
    CField& area = mesh->create_field(Mesh::Tags::area(),CField::Basis::FACE_BASED,"P0");
    area.add_tag(Mesh::Tags::area());
    CLoop::Ptr compute_area = create_component_ptr< CForAllFaces >("compute_area");
    compute_area->configure_property("regions", std::vector<URI>(1,area.topology().uri()));
    compute_area->create_loop_operation("CF.Solver.Actions.CComputeArea");
    configure_option_recursively(Mesh::Tags::area(),area.uri());
    compute_area->execute();
    remove_component(compute_area->name());
  }

  // create/initialize a solution if it is not available
  CField::Ptr solution_ptr = find_component_ptr_with_name<CField>(*mesh,"solution");
  if ( is_null(solution_ptr) )
  {
    ///@todo get variable names etc, from Physics
    CFinfo <<  "  Creating field \"solution\", cellbased, with vars rho[1],rhoU["+to_str(m_physical_model.lock()->dimensions())+"],rhoE[1]" << CFendl;
    CField& solution = mesh->create_field("solution",CField::Basis::CELL_BASED,"P0","rho[1],rhoU["+to_str(m_physical_model.lock()->dimensions())+"],rhoE[1]");
    solution.add_tag("solution");
  }

  CField& solution = find_component_with_name<CField>(*mesh,"solution");
  m_solution->link_to(solution.self());

  Component::Ptr residual_ptr = find_component_ptr_with_tag(*mesh,"residual");
  if ( is_null(residual_ptr) )
  {
    CFinfo << "  Creating field \"residual\", cellbased" << CFendl;
    residual_ptr = mesh->create_field("residual",solution).self();
    residual_ptr->add_tag("residual");
  }
  m_residual->link_to(residual_ptr);

  Component::Ptr wave_speed_ptr = find_component_ptr_with_name(*mesh,"wave_speed");
  if ( is_null(wave_speed_ptr) )
  {
    CFinfo << "  Creating field \"wave_speed\", cellbased" << CFendl;
    wave_speed_ptr = mesh->create_scalar_field("wave_speed",solution).self();
    wave_speed_ptr->add_tag("wave_speed");
  }
  m_wave_speed->link_to(wave_speed_ptr);

  Component::Ptr update_coeff_ptr = find_component_ptr_with_name(*mesh,"update_coeff");
  if ( is_null(update_coeff_ptr) )
  {
    CFinfo << "  Creating field \"update_coeff\", cellbased" << CFendl;
    update_coeff_ptr = mesh->create_scalar_field("update_coeff",solution).self();
    update_coeff_ptr->add_tag("update_coeff");
  }
  m_update_coeff->link_to(update_coeff_ptr);

  auto_config_fields(*this);

  access_component("iterate/2_compute_rhs/2.1_init_residual").configure_property("field",residual_ptr->uri());
  access_component("iterate/2_compute_rhs/2.2_init_wave_speed").configure_property("field",wave_speed_ptr->uri());
}

//////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::trigger_time()
{

  URI time_uri = property("time").value<URI>();
  Component::Ptr time_ptr = access_component_ptr(time_uri);
  if (is_null(time_ptr))
  {
    CFinfo << "No time component found at [" << time_uri.path() << "]. Performing steady state computations." << CFendl;
  }
  else
  {
    m_time = time_ptr->as_ptr_checked<CTime>();
    m_iterate->configure_option_recursively("time",m_time.lock()->uri());
    m_iterate->configure_option_recursively("time_accurate",true);
  }

}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::trigger_physical_model()
{

  URI uri = property("physical_model").value<URI>();
  Component::Ptr ptr = access_component_ptr(uri);
  if (is_null(ptr))
  {
    throw SetupError (FromHere(),"Physical Model not found in ["+uri.path()+"]");
  }
  else
  {
    m_physical_model = ptr->as_ptr_checked<CPhysicalModel>();
  }

}

//////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::solve()
{
  if ( m_physical_model.expired() == true )
    trigger_physical_model();

  if ( m_domain.expired() == true )
    trigger_domain();

  if ( m_time.expired() == true )
    trigger_time();

  m_iterate->execute();
}

////////////////////////////////////////////////////////////////////////////////

CAction& FiniteVolumeSolver::create_bc(const std::string& name, const std::vector<CRegion::Ptr>& regions, const std::string& bc_builder_name)
{
  std::vector<URI> regions_uri; regions_uri.reserve(regions.size());
  boost_foreach(CRegion::Ptr region, regions)
    regions_uri.push_back(region->uri());

  CAction::Ptr for_all_faces = m_apply_bcs->create_component_ptr<CForAllFaces>(name);
  for_all_faces->configure_property("regions",regions_uri);
  CAction& bc = for_all_faces->create_action(bc_builder_name,bc_builder_name);
  auto_config_fields(bc);
  return bc;
}

////////////////////////////////////////////////////////////////////////////////

CAction& FiniteVolumeSolver::create_bc(const std::string& name, const CRegion& region, const std::string& bc_builder_name)
{
  CAction::Ptr for_all_faces = m_apply_bcs->create_component_ptr<CForAllFaces>(name);
  for_all_faces->configure_property("regions",std::vector<URI>(1,region.uri()));
  CAction& bc = for_all_faces->create_action(bc_builder_name,bc_builder_name);
  auto_config_fields(bc);
  return bc;
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::auto_config_fields(Component& parent)
{
  if ( m_domain.expired() == true )
    trigger_domain();

  CMesh& mesh = find_component_recursively<CMesh>(*m_domain.lock());

  boost_foreach(CField& field, find_components<CField>(mesh) )
  {
    parent.configure_option_recursively(field.name(), field.uri());
  }
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::signal_create_bc( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.option<std::string>("name");
  std::string builder = options.option<std::string>("builder");
  std::vector<URI> regions_uri = options.array<URI>("regions");
  std::vector<CRegion::Ptr> regions(regions_uri.size());
  for (Uint i=0; i<regions_uri.size(); ++i)
  {
    regions[i] = access_component(regions_uri[i]).as_ptr_checked<CRegion>();
  }
  create_bc(name,regions,builder);
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolumeSolver::signature_create_bc( SignalArgs& node )
{
  SignalOptions options( node );

  // name
  options.add<std::string>("name", std::string(), "Name for created boundary term" );

  // type
  CFactory::Ptr bc_factory = Common::Core::instance().factories().get_factory<BC>();
  std::vector<std::string> bcs;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *bc_factory ) )
      bcs.push_back(bdr.name());

  // create de value and add the restricted list
  options.add<std::string>( "builder", std::string() , "Choose BC", bcs, " ; ");

  // regions
  std::vector<URI> dummy;
  // create here the list of restricted surface regions
  options.add("regions", dummy , "Regions where to apply the boundary condition", " ; " );

}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF
