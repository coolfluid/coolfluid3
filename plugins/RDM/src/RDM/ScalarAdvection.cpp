// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"

#include "Solver/CModelSteady.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CSolver.hpp"


#include "RDM/ScalarAdvection.hpp"

namespace CF {
namespace RDM {

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;

Common::ComponentBuilder < ScalarAdvection, Component, LibRDM > ScalarAdvection_Builder;

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::ScalarAdvection ( const std::string& name  ) :
  Component ( name )
{
  // signals

  this->regist_signal ( "create_model" , "Creates a scalar advection model", "Create Model" )->signal->connect ( boost::bind ( &ScalarAdvection::signal_create_model, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  signal("create_model")->signature->connect( boost::bind( &ScalarAdvection::signature_create_model, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::~ScalarAdvection()
{
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::signal_create_model ( Common::SignalArgs& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  std::string name  = options.get_option<std::string>("ModelName");

  CModel::Ptr model = Core::instance().root()->create_component<CModelSteady>( name );

  // create the Physical Model
  CPhysicalModel::Ptr pm = model->create_component<CPhysicalModel>("Physics");
  pm->mark_basic();

  std::string phys  = options.get_option<std::string>("PhysicalModel");

  pm->configure_property( "Type", phys );
  pm->configure_property( "DOFs", 1u );
  pm->configure_property( "Dimensions", 2u );

  model->create_domain( "Domain" );

  // setup iterative solver
  CSolver::Ptr solver = create_component_abstract_type<CSolver>("CF.RDM.RKRD", "Solver");
  solver->mark_basic();
  model->add_component( solver );
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::signature_create_model( SignalArgs& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  options.set_option<std::string>("ModelName", std::string(), "Name for created model" );
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
