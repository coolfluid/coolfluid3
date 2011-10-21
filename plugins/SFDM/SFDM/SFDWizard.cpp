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
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/GroupActions.hpp"
#include "common/Signal.hpp"

#include "common/XML/SignalOptions.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/FlowSolver.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CTime.hpp"

#include "Solver/Actions/CAdvanceTime.hpp"
#include "Solver/Actions/CCriterionTime.hpp"
#include "Solver/Actions/CForAllCells.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Region.hpp"

#include "mesh/actions/InitFieldConstant.hpp"
#include "mesh/actions/InitFieldFunction.hpp"
#include "mesh/actions/BuildFaces.hpp"
#include "mesh/actions/BuildVolume.hpp"
#include "mesh/actions/CreateSpaceP0.hpp"

#include "SFDM/SFDWizard.hpp"
#include "SFDM/UpdateSolution.hpp"
#include "SFDM/ComputeUpdateCoefficient.hpp"
#include "SFDM/OutputIterationInfo.hpp"
#include "SFDM/ComputeRhsInCell.hpp"
#include "SFDM/CreateSpace.hpp"
#include "SFDM/CreateSFDFields.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

  using namespace common;
  using namespace common::XML;

  using namespace mesh;
  using namespace Solver;
  using namespace Solver::Actions;
  using namespace mesh::actions;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SFDWizard, CWizard, LibSFDM> SFDWizard_Builder;

//////////////////////////////////////////////////////////////////////////////

SFDWizard::SFDWizard( const std::string& name )
: CWizard(name)
{

  m_properties["brief"] = std::string("MISSING");
  m_properties["description"] = std::string("MISSING");

  m_options.add_option( OptionT<std::string>::create("model", "SFD_simulation") )
    ->description("Name to give to the simulation model")
    ->pretty_name("Model Name")
    ->mark_basic();

  m_options.add_option( OptionT<Uint>::create("dim", 1u) )
    ->description("Dimension of the simulation")
    ->pretty_name("Dimension")
    ->mark_basic();

  //options().add_option( OptionT<std::string>::create("physics", "CF.") )
  //  ->description("Builder name for the physical model")
  //  ->pretty_name("Physics")
  //  ->mark_basic();

  m_options.add_option( OptionT<std::string>::create("solution_state", "CF.Euler.Cons1D") )
    ->description("Solution state builder")
    ->pretty_name("Solution State")
    ->mark_basic();

  m_options.add_option( OptionT<std::string>::create("roe_state", "CF.Euler.Roe1D") )
    ->description("Roe state builder")
    ->pretty_name("Roe State")
    ->mark_basic();

  m_options.add_option( OptionT<Uint>::create("P", 0u) )
    ->description("The order of the polynomial of the solution")
    ->pretty_name("Polynomial Order")
    ->mark_basic();

  m_options.add_option( OptionT<Uint>::create("RK_stages", 2u) )
    ->description("The number of Runge Kutta stages")
    ->pretty_name("Runge Kutta stages")
    ->mark_basic();

  m_options.add_option( OptionT<Real>::create(FlowSolver::Tags::cfl(), 1.) )
    ->description("The Courant-Friedrichs-Lax Number")
    ->pretty_name("CFL")
    ->mark_basic();

  m_options.add_option( OptionT<bool>::create(FlowSolver::Tags::time_accurate(), true) )
    ->description("Time accurate or steady state")
    ->pretty_name("Time Accurate")
    ->mark_basic();

  m_options.add_option( OptionT<bool>::create("output_file", "mesh_t${time}.msh") )
    ->description("File to write")
    ->pretty_name("Output File")
    ->mark_basic();

  regist_signal( "create_simulation" )
    ->description("Create a simulation")
    ->pretty_name("Create Simulation")
    ->connect ( boost::bind ( &SFDWizard::signal_create_simulation, this, _1 ) )
    ->signature( boost::bind ( &SFDWizard::signature_create_simulation, this, _1) );

  SignalPtr sig_prepare_simulation = regist_signal( "prepare_simulation" )
    ->description("Prepare a simulation")
    ->pretty_name("Prepare Simulation")
    ->connect( boost::bind ( &SFDWizard::signal_prepare_simulation, this, _1 ) )
    ->signature( boost::bind ( &SFDWizard::signature_prepare_simulation, this, _1) );

  SignalPtr sig_init_sol = regist_signal( "initialize_solution" )
    ->description("Initialize Solution")
    ->pretty_name("Initialize Solution")
    ->connect( boost::bind ( &SFDWizard::signal_initialize_solution, this, _1 ) )
    ->signature( boost::bind ( &SFDWizard::signature_initialize_solution, this, _1) );

  SignalPtr sig_start_simulation = regist_signal( "start_simulation" )
    ->description("Start a simulation")
    ->pretty_name("Start Simulation")
    ->connect ( boost::bind ( &SFDWizard::signal_start_simulation, this, _1 ) )
    ->signature( boost::bind ( &SFDWizard::signature_start_simulation, this, _1) );

  m_model_link = create_static_component_ptr<Link>("current_model");
}

/////////////////////////////////////////////////////////////////////////////

CModelUnsteady& SFDWizard::model()
{
  return m_model_link->follow()->as_type<CModelUnsteady>();
}

/////////////////////////////////////////////////////////////////////////////

void SFDWizard::create_simulation()
{
  CModelUnsteady& model = Core::instance().root().create_component<CModelUnsteady>(option("model").value_str());
  m_model_link->link_to(model);

  CPhysicalModel& physical_model = model.create_physics("Physics");

  /// @todo should be setup differently
  physical_model.configure_option("solution_state",option("solution_state").value_str());

  Domain& domain                = model.create_domain("Domain");
  CTime& time                    = model.create_time("Time");
  CSolver& solver                = model.create_solver("CF.Solver.FlowSolver");

  // These 2 functions are the only specific ones to SFDM (together with some configuration options)
  // -------------------------
  build_solve();
  build_setup();
  // -------------------------

  solver.configure_option(FlowSolver::Tags::physical_model(),physical_model.uri());
  solver.configure_option(FlowSolver::Tags::time(),time.uri());

  model.tools().create_component_ptr<InitFieldFunction>("initialize_solution");

  CFinfo << "\nCreate the mesh in ["<<domain.uri().path()<<"] and call \""<<uri().path()<<"/prepare\"" << CFendl;
}

void SFDWizard::prepare_simulation()
{
  if (m_model_link->is_linked() == false)
    throw SetupError(FromHere(),"Model was not created through this wizard");

  CModel& model = m_model_link->follow()->as_type<CModel>();

  std::vector<Mesh::Ptr> meshes = find_components<Mesh>(model.domain()).as_vector();
  if (meshes.size() == 0)
    throw SetupError(FromHere(),"Mesh was not added to the domain");

  if (meshes.size() > 1)
  {
    if (options().check("mesh"))
    {
      model.solver().configure_option("mesh",option("mesh").value<URI>());
    }
    else
    {
      m_options.add_option( OptionURI::create("mesh","Mesh","The mesh to solve on",model.domain().uri()/URI("mesh")) )->mark_basic();
      throw SetupError(FromHere(),"Multiple meshes exist in ["+model.domain().uri().path()+"]\n"
                       "Set the option \"mesh\" to specify the path to the mesh");
    }
  }
  else
  {
    model.solver().configure_option("mesh",meshes[0]->uri());
  }

  CFinfo << CFendl;
  CFinfo << "To add Boundary Conditions:   " << "call " << model.solver().uri().path() << "/create_bc_action" << CFendl;
  CFinfo << "To add Inner domain actions:  " << "call " << model.solver().uri().path() << "/create_inner_action" << CFendl;
  CFinfo << "To initialize solution:       " << model.tools().get_child("initialize_solution").uri().path() << "     (needs configuring) " << CFendl;
  CFinfo << "                    or:       " << "call " << uri().path() << "/initialize_solution" << CFendl;
  CFinfo << CFendl;
  CFinfo << "To start simulation:          " << "call " << uri().path() << "/start_simulation" << CFendl;

}

//////////////////////////////////////////////////////////////////////////////

void SFDWizard::start_simulation(const Real& end_time, const Real& time_step)
{
  if (m_model_link->is_linked() == false)
    throw SetupError(FromHere(),"Model was not created through this wizard");

  CModelUnsteady& model = m_model_link->follow()->as_type<CModelUnsteady>();

  model.time()
      .configure_option("end_time",end_time)
      .configure_option("time_step",time_step);

  model.solver().configure_option_recursively(FlowSolver::Tags::cfl(),option(FlowSolver::Tags::cfl()).value<Real>());
  model.solver().configure_option_recursively(FlowSolver::Tags::time_accurate(),option(FlowSolver::Tags::time_accurate()).value<bool>());

  model.simulate();
}

//////////////////////////////////////////////////////////////////////////////

void SFDWizard::initialize_solution(const std::vector<std::string>& functions)
{
  Action& init_solution = model().tools().get_child("initialize_solution").as_type<Action>();
  init_solution.configure_option("functions",functions);
  init_solution.execute();
}


//////////////////////////////////////////////////////////////////////////////

void SFDWizard::signal_create_simulation( SignalArgs& node )
{
  create_simulation();
}

////////////////////////////////////////////////////////////////////////////////

void SFDWizard::signature_create_simulation( SignalArgs& node )
{
}

//////////////////////////////////////////////////////////////////////////////

void SFDWizard::signal_prepare_simulation( SignalArgs& node )
{
  prepare_simulation();
}

////////////////////////////////////////////////////////////////////////////////

void SFDWizard::signature_prepare_simulation( SignalArgs& node )
{
}

//////////////////////////////////////////////////////////////////////////////

void SFDWizard::signal_initialize_solution( SignalArgs& node )
{
  SignalOptions options( node );

  std::vector<std::string> functions =
      options.array<std::string>("functions");
  initialize_solution(functions);
}

////////////////////////////////////////////////////////////////////////////////

void SFDWizard::signature_initialize_solution( SignalArgs& node )
{
  SignalOptions options( node );

  options.add_option< OptionArrayT<std::string> >("functions", std::vector<std::string>() )
     ->description("Analytical functions (x,y,z)");
}

//////////////////////////////////////////////////////////////////////////////

void SFDWizard::signal_start_simulation( SignalArgs& node )
{
  SignalOptions options( node );

  if (options.exists("current_time"))
    m_model_link->follow()->as_type<CModel>().configure_option("time",options.option<Real>("time"));

  Real end_time = 0;
  if (options.exists("end_time"))
    end_time = options.option<Real>("end_time");

  if (options.exists("time_step"))
  {
    const Real time_step = options.option<Real>("time_step");
    start_simulation(end_time,time_step);
  }
  else
  {
    start_simulation(end_time);
  }
}

////////////////////////////////////////////////////////////////////////////////

void SFDWizard::signature_start_simulation( SignalArgs& node )
{
  SignalOptions options( node );

  // name
  CTime& time = m_model_link->follow()->as_type<CModelUnsteady>().time();
  options.add_option< OptionT<Real> >("current_time", time.time())
     ->description("Current Time" );
  options.add_option< OptionT<Real> >("end_time", time.option("end_time").value<Real>())
     ->description("End Time" );
  options.add_option< OptionT<Real> >("time_step", time.option("time_step").value<Real>())
     ->description("Time Step" );
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Here starts the solver itself. This could be customized later for
// different algorithms. Could be another wizard.
//////////////////////////////////////////////////////////////////////////////

void SFDWizard::build_solve()
{
  FlowSolver& solver = m_model_link->follow()->as_type<CModel>().solver().as_type<FlowSolver>();

  Component& iterate = solver.create_solve("iterate","CF.Solver.Actions.CIterate");
  Component& RK = iterate.create_component("1_RK_stages","CF.RungeKutta.RK");
  RK.configure_option("stages",option("RK_stages").value<Uint>());
  Component& compute_rhs = RK.access_component("1_for_each_stage/1_pre_update_actions").create_component<GroupActions>("1_compute_rhs").mark_basic();
  compute_rhs.add_tag(FlowSolver::Tags::inner());
  compute_rhs.create_component <InitFieldConstant>("1.1_init_residual")
    .mark_basic()
    .configure_option("constant",0.)
    .option("field").add_tag(FlowSolver::Tags::residual());

  compute_rhs.create_static_component<InitFieldConstant>("1.2_init_wave_speed")
    .mark_basic()
    .configure_option("constant",0.)
    .option("field").add_tag(FlowSolver::Tags::wave_speed());

  Component& for_all_cells =
    compute_rhs.create_component<CForAllCells>("1.3_for_all_cells").mark_basic();
  Component& compute_rhs_in_cell = for_all_cells.create_component<ComputeRhsInCell>("1.3.1_compute_rhs_in_cell").mark_basic();

  RK.access_component("1_for_each_stage/1_pre_update_actions").create_component<ComputeUpdateCoefficient>("2_compute_update_coeff").mark_basic();
  iterate.create_component<OutputIterationInfo>("2_output_info").mark_basic();
  iterate.create_component<CCriterionTime>("time_stop_criterion").mark_basic();

  solver.configure_option_recursively("riemann_solver",std::string("CF.RiemannSolvers.Roe"));
  solver.configure_option_recursively("roe_state",option("roe_state").value_str());

  solver.configure_option_recursively("solution_state",m_model_link->follow()->as_type<CModel>().physics().solution_state().uri());

}

//////////////////////////////////////////////////////////////////////////////

void SFDWizard::build_setup()
{
  /// Create a Solver::Action that gets executed automatically when the FlowSolver
  /// has been configured with ALL of the following:
  /// - physical_model
  /// - mesh
  /// - time

  FlowSolver& solver = model().solver().as_type<FlowSolver>();
  Action& setup = solver.as_type<FlowSolver>().create_setup(FlowSolver::Tags::setup(),"CF.SFDM.SFDSetup");

  /// Create a mesh transformer to adapt the mesh for SFDM
  MeshTransformer& transform_mesh = setup.create_component<MeshTransformer>("1_transform_mesh").mark_basic().as_type<MeshTransformer>();
  transform_mesh.create_component<BuildFaces>       ("1_build_faces").mark_basic().configure_option("store_cell2face",true);
  transform_mesh.create_component<CreateSpaceP0>     ("2_create_space_P0").mark_basic();
  transform_mesh.create_component<SFDM::CreateSpace> ("3_create_sfd_spaces").mark_basic().configure_option("P",option("P").value<Uint>());
  transform_mesh.create_component<BuildVolume>      ("4_build_volume_field").mark_basic();

  /// Create an action that creates all fields used for SFDM
  setup.create_component<CreateSFDFields>("2_create_sfd_fields");
}

//////////////////////////////////////////////////////////////////////////////

ComponentBuilder<SFDSetup,Action,LibSFDM> SFDSetup_builder;

void SFDSetup::execute()
{
  /// This gets executed AUTOMATICALLY when the FlowSolver has been configured with ALL of the following:
  /// - physical_model
  /// - mesh
  /// - time

  /// 1) execute all mesh transforming actions, and field creation (added in SFDWizard::build_setup())
  boost_foreach(Action& action, find_components<Action>(*this))
    action.execute();

  /// @todo configure this differently perhaps
  /// 2) set looping regions to the entire mesh
  access_component("../iterate/1_RK_stages/1_for_each_stage/1_pre_update_actions/1_compute_rhs/1.3_for_all_cells").configure_option("regions",std::vector<URI>(1,mesh().topology().uri()));

  /// 3) configure the initialize_solution component. The field must be set to the solution.
  access_component("../../tools/initialize_solution").configure_option("field",mesh().get_child(FlowSolver::Tags::solution()).uri());
}

} // SFDM
} // CF
