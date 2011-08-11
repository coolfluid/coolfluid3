// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

// supported physical models

#include "Physics/Scalar/Scalar2D.hpp"
#include "Physics/Scalar/ScalarSys2D.hpp"
#include "Physics/Scalar/Scalar3D.hpp"
#include "Physics/NavierStokes/NavierStokes2D.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CTime.hpp"
#include "RDM/Tags.hpp"

#include "Solver/Actions/CCriterionTime.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"
#include "RDM/TimeStepping.hpp"
#include "RDM/Reset.hpp"
#include "RDM/RK.hpp"
#include "RDM/SetupMultipleSolutions.hpp"
#include "RDM/ComputeDualArea.hpp"

#include "UnsteadyExplicit.hpp"

namespace CF {
namespace RDM {

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Physics;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < UnsteadyExplicit, CF::Solver::CWizard, LibRDM > UnsteadyExplicit_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

UnsteadyExplicit::UnsteadyExplicit ( const std::string& name  ) :
  CF::Solver::CWizard ( name )
{
  // options

  m_options.add_option< OptionT<Uint> >( "rkorder", 1u )
      ->description("Order of the explicit time stepping")
      ->pretty_name("Time Step Order");

  // signals

  regist_signal( "create_model" )
    ->connect( boost::bind( &UnsteadyExplicit::signal_create_model, this, _1 ) )
    ->description("Creates a model for solving steady problms with RD using explicit iterations")
    ->pretty_name("Create Model");

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);

  signal("create_model")->signature( boost::bind( &UnsteadyExplicit::signature_create_model, this, _1));
}


UnsteadyExplicit::~UnsteadyExplicit() {}


CModel& UnsteadyExplicit::create_model( const std::string& model_name, const std::string& physics_builder )
{

  const Uint rkorder = option("rkorder").value<Uint>();

  // (1) create the model

  CModel& model = Common::Core::instance().root().create_component<CModelUnsteady>( model_name );

  // (2) create the domain

  CDomain& domain = model.create_domain( "Domain" );

  // (3) create the Physical Model

  PhysModel& pm = model.create_physics( physics_builder );

  pm.mark_basic();

  // (4) setup solver

  CF::RDM::RDSolver& solver = model.create_solver( "CF.RDM.RDSolver" ).as_type< CF::RDM::RDSolver >();

  solver.mark_basic();

  solver.properties().add_property("rkorder", rkorder); // place it somewhere for other components to access

  // (4a) setup time step stop condition

  CCriterionTime& time_limit = solver.time_stepping().create_component<CCriterionTime>("TimeLimit");

  time_limit.configure_option( RDM::Tags::time(), solver.time_stepping().time().uri() );

  // (4b) setup iterative solver reset action

  Reset::Ptr reset  = allocate_component<Reset>("Reset");
  solver.iterative_solver().pre_actions().append( reset );

  /// @todo this reset configuation must be corrected

  std::vector<std::string> reset_fields;
  reset_fields.push_back( RDM::Tags::residual() );
  reset_fields.push_back( RDM::Tags::wave_speed() );
  reset->configure_option("FieldTags", reset_fields);

  // (4c) setup iterative solver explicit time stepping  - RK

  RK::Ptr rk = allocate_component<RK>("Step");
  solver.iterative_solver().update().append( rk );

  solver.iterative_solver().get_child("MaxIterations").configure_option("maxiter", rkorder); // eg: 2nd order -> 2 rk iterations

  // (4d) setup solver fields

  /// @todo add here any other actions to allocte more fields or storage

  SetupMultipleSolutions::Ptr setup = allocate_component<SetupMultipleSolutions>("SetupFields");
  solver.prepare_mesh().append(setup);

  setup->configure_option( "nb_levels", rkorder );

  ComputeDualArea::Ptr dual_area = allocate_component<ComputeDualArea>("ComputeDualArea");
  solver.prepare_mesh().append(dual_area);



  // (5) configure domain, physical model and solver in all subcomponents

  solver.configure_option_recursively( RDM::Tags::domain(),         domain.uri() );
  solver.configure_option_recursively( RDM::Tags::physical_model(), pm.uri() );
  solver.configure_option_recursively( RDM::Tags::solver(),         solver.uri() );

  return model;
}


void UnsteadyExplicit::signal_create_model ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  std::string model_name  = options.value<std::string>("model_name");
  std::string phys  = options.value<std::string>("physical_model");

  create_model( model_name, phys );
}


void UnsteadyExplicit::signature_create_model( SignalArgs& node )
{
  SignalOptions options( node );

  options.add_option< OptionT<std::string> >("model_name", std::string() )
      ->description("Name for created model" )
      ->pretty_name("Model Name");

  std::vector<boost::any> models = boost::assign::list_of
      ( Scalar::Scalar2D::type_name() )
      ( Scalar::Scalar3D::type_name() )
      ( Scalar::ScalarSys2D::type_name() )
      ( NavierStokes::NavierStokes2D::type_name() ) ;

  options.add_option< OptionT<std::string> >("physical_model", std::string() )
      ->description("Name of the Physical Model")
      ->pretty_name("Physical Model Type")
      ->restricted_list() = models;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
