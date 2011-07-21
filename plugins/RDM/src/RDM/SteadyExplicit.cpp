// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/WriteMesh.hpp"

#include "Solver/CModelSteady.hpp"
#include "Solver/CSolver.hpp"

#include "RDM/SteadyExplicit.hpp"
#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"
#include "RDM/TimeStepping.hpp"
#include "RDM/FwdEuler.hpp"

// supported physical models

#include "Physics/Scalar/Scalar2D.hpp"
#include "Physics/Scalar/ScalarSys2D.hpp"
#include "Physics/Scalar/Scalar3D.hpp"
#include "Physics/NavierStokes/NavierStokes2D.hpp"

namespace CF {
namespace RDM {

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Physics;
using namespace CF::Solver;

Common::ComponentBuilder < SteadyExplicit, CF::Solver::CWizard, LibRDM > SteadyExplicit_Builder;

////////////////////////////////////////////////////////////////////////////////

SteadyExplicit::SteadyExplicit ( const std::string& name  ) :
  CF::Solver::CWizard ( name )
{
  // signals

  regist_signal( "create_model" )
    ->connect( boost::bind( &SteadyExplicit::signal_create_model, this, _1 ) )
    ->description("Creates a model for solving steady problms with RD using explicit iterations")
    ->pretty_name("Create Model");

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);

  signal("create_model")->signature( boost::bind( &SteadyExplicit::signature_create_model, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

SteadyExplicit::~SteadyExplicit() {}


CModel& SteadyExplicit::create_model( const std::string& model_name, const std::string& physics_builder )
{
  // create the model

  CModel& model = Common::Core::instance().root().create_component<CModelSteady>( model_name );

  // create the domain

  CDomain& domain = model.create_domain( "Domain" );

  // create the Physical Model

  PhysModel& pm = model.create_physics( physics_builder );

  pm.mark_basic();

  // setup iterative solver

  CF::RDM::RDSolver& solver = model.create_solver( "CF.RDM.RDSolver" ).as_type< CF::RDM::RDSolver >();

  solver.mark_basic();

  // explicit time stepping  - forward euler

  solver.iterative_solver().update()
      .append( allocate_component<FwdEuler>("Step") );

  solver.configure_option_recursively( RDM::Tags::domain(),         domain.uri() );
  solver.configure_option_recursively( RDM::Tags::physical_model(), pm.uri() );
  solver.configure_option_recursively( RDM::Tags::solver(),         solver.uri() );

//  solver.time_stepping().configure_option_recursively( "time_step", Real(0) );
//  solver.time_stepping().configure_option_recursively( "end_time",  Real(0) );

  solver.time_stepping().configure_option_recursively( "maxiter",   1u);

  return model;
}


void SteadyExplicit::signal_create_model ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  std::string model_name  = options.value<std::string>("model_name");
  std::string phys  = options.value<std::string>("physical_model");

  create_model( model_name, phys );
}

////////////////////////////////////////////////////////////////////////////////

void SteadyExplicit::signature_create_model( SignalArgs& node )
{
  SignalOptions options( node );

  options.add_option< OptionT<std::string> >("model_name", std::string() )
      ->set_description("Name for created model" )
      ->set_pretty_name("Model Name");

  std::vector<boost::any> models = boost::assign::list_of
      ( Scalar::Scalar2D::type_name() )
      ( Scalar::Scalar3D::type_name() )
      ( Scalar::ScalarSys2D::type_name() )
      ( NavierStokes::NavierStokes2D::type_name() ) ;

  options.add_option< OptionT<std::string> >("physical_model", std::string() )
      ->set_description("Name of the Physical Model")
      ->set_pretty_name("Physical Model Type")
      ->restricted_list() = models;
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
