// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"

#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "common/XML/SignalOptions.hpp"

#include "mesh/MeshReader.hpp"
#include "mesh/Domain.hpp"
#include "mesh/WriteMesh.hpp"

// supported physical models

#include "Physics/Scalar/Scalar2D.hpp"
#include "Physics/Scalar/ScalarSys2D.hpp"
#include "Physics/Scalar/Scalar3D.hpp"
#include "Physics/NavierStokes/NavierStokes2D.hpp"
#include "Physics/LinEuler/LinEuler2D.hpp"

#include "solver/ModelUnsteady.hpp"
#include "solver/Time.hpp"
#include "RDM/Tags.hpp"

#include "solver/actions/CriterionTime.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"
#include "RDM/TimeStepping.hpp"
#include "RDM/Reset.hpp"
#include "RDM/RK.hpp"
#include "RDM/CopySolution.hpp"
#include "RDM/SetupMultipleSolutions.hpp"
#include "RDM/ComputeDualArea.hpp"

#include "UnsteadyExplicit.hpp"

namespace cf3 {
namespace RDM {

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;
using namespace cf3::solver::actions;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < UnsteadyExplicit, cf3::solver::Wizard, LibRDM > UnsteadyExplicit_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

UnsteadyExplicit::UnsteadyExplicit ( const std::string& name  ) :
  cf3::solver::Wizard ( name )
{
  // options

  options().add( "rkorder", 1u )
      .description("Order of the explicit time stepping")
      .pretty_name("Time Step Order");

  // signals

  regist_signal( "create_model" )
    .connect( boost::bind( &UnsteadyExplicit::signal_create_model, this, _1 ) )
    .description("Creates a model for solving steady problms with RD using explicit iterations")
    .pretty_name("Create Model");

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);

  signal("create_model")->signature( boost::bind( &UnsteadyExplicit::signature_create_model, this, _1));
}


UnsteadyExplicit::~UnsteadyExplicit() {}


Model& UnsteadyExplicit::create_model( const std::string& model_name, const std::string& physics_builder )
{

  const Uint rkorder = options().value<Uint>("rkorder");

  // (1) create the model

  Model& model = *common::Core::instance().root().create_component<ModelUnsteady>( model_name );

  // (2) create the domain

  Domain& domain = model.create_domain( "Domain" );

  // (3) create the Physical Model

  PhysModel& pm = model.create_physics( physics_builder );

  pm.mark_basic();

  // (4) setup solver

  cf3::RDM::RDSolver& solver = *model.create_solver( "cf3.RDM.RDSolver" ).handle< cf3::RDM::RDSolver >();

  solver.mark_basic();

  solver.properties().add("rkorder", rkorder); // place it somewhere for other components to access

  // (4a) setup time step stop condition

  CriterionTime& time_limit = *solver.time_stepping().create_component<CriterionTime>("TimeLimit");

  time_limit.options().set( RDM::Tags::time(), solver.time_stepping().time().handle<Time>() /* .uri()*/ );

  // (4b) setup iterative solver reset action

  solver.time_stepping().pre_actions().create_component<CopySolution>("CopySolution");

  // (4b) setup iterative solver reset action

  Handle<Reset> reset(solver.iterative_solver().pre_actions().create_component<Reset>("Reset"));

  std::vector<std::string> reset_fields;
  reset_fields.push_back( RDM::Tags::residual() );
  reset_fields.push_back( RDM::Tags::wave_speed() );
  reset->options().set("FieldTags", reset_fields);

  // (4c) setup iterative solver explicit time stepping  - RK
  solver.iterative_solver().update().create_component<RK>("Step");

  solver.iterative_solver().get_child("MaxIterations")->options().set("maxiter", rkorder); // eg: 2nd order -> 2 rk iterations

  solver.iterative_solver().get_child("PostActions")->get_child("IterationSummary")->options().set("print_rate", 0u); // dont print under unsteady iterations

  // (4d) setup solver fields
  solver.prepare_mesh().create_component<SetupMultipleSolutions>("SetupFields")->options().set( "nb_levels", rkorder );
  solver.prepare_mesh().create_component<ComputeDualArea>("ComputeDualArea");

  // (5) configure domain, physical model and solver in all subcomponents

  solver.configure_option_recursively( RDM::Tags::domain(),         domain.uri() );
  solver.configure_option_recursively( RDM::Tags::physical_model(), pm.handle<PhysModel>() );
  solver.configure_option_recursively( RDM::Tags::solver(),         solver.handle<Solver>() );

  return model;
}


void UnsteadyExplicit::signal_create_model ( common::SignalArgs& node )
{
  SignalOptions options( node );

  std::string model_name  = options.value<std::string>("model_name");
  std::string phys  = options.value<std::string>("physical_model");

  create_model( model_name, phys );
}


void UnsteadyExplicit::signature_create_model( SignalArgs& node )
{
  SignalOptions options( node );

  options.add("model_name", std::string() )
      .description("Name for created model" )
      .pretty_name("Model Name");

  std::vector<boost::any> models = boost::assign::list_of
      ( Scalar::Scalar2D::type_name() )
      ( Scalar::Scalar3D::type_name() )
      ( Scalar::ScalarSys2D::type_name() )
      ( NavierStokes::NavierStokes2D::type_name() )
      ( LinEuler::LinEuler2D::type_name() ) ;

  options.add("physical_model", std::string() )
      .description("Name of the Physical Model")
      .pretty_name("Physical Model Type")
      .restricted_list() = models;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
