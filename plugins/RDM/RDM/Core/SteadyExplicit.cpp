// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "boost/assign/list_of.hpp"

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"


#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"

#include "Solver/CModelSteady.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CSolver.hpp"

#include "RDM/Core/SteadyExplicit.hpp"

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

Common::ComponentBuilder < SteadyExplicit, Solver::CWizard, LibCore > SteadyExplicit_Builder;

////////////////////////////////////////////////////////////////////////////////

SteadyExplicit::SteadyExplicit ( const std::string& name  ) :
  Solver::CWizard ( name )
{
  // signals

  this->regist_signal ( "create_model" , "Creates a scalar advection model", "Create Model" )->signal->connect ( boost::bind ( &SteadyExplicit::signal_create_model, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  signal("create_model")->signature->connect( boost::bind( &SteadyExplicit::signature_create_model, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

SteadyExplicit::~SteadyExplicit()
{
}

////////////////////////////////////////////////////////////////////////////////

void SteadyExplicit::signal_create_model ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  std::string name  = options.option<std::string>("ModelName");

  CModel& model = Core::instance().root().create_component<CModelSteady>( name );

  // create the domain

  model.create_domain( "Domain" );

  // create the Physical Model

  std::string phys  = options.option<std::string>("PhysicalModel");

  PhysModel::Ptr pm = build_component_abstract_type<PhysModel>( phys, "Physics");
  pm->mark_basic();

  model.add_component(pm);

  // setup iterative solver

  CSolver::Ptr solver = build_component_abstract_type_reduced<CSolver>( "RKRD", "Solver");
  solver->mark_basic();

  model.add_component( solver );

  solver->configure_option("physics", pm->uri() );
}

////////////////////////////////////////////////////////////////////////////////

void SteadyExplicit::signature_create_model( SignalArgs& node )
{
  SignalOptions options( node );

  options.add<std::string>("ModelName", std::string(), "Name for created model" );

  std::vector<std::string> models = boost::assign::list_of
      ( Scalar::Scalar2D::type_name() )
      ( Scalar::Scalar3D::type_name() )
      ( Scalar::ScalarSys2D::type_name() )
      ( NavierStokes::NavierStokes2D::type_name() ) ;

  options.add<std::string>("PhysicalModel", std::string(), "Name of the Physical Model", models, " ; ");

}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
