// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CFactory.hpp"
#include "Common/OptionT.hpp"
 

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CDomain.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/CModelSteady.hpp"
#include "Solver/CPhysicalModel.hpp"

#include "LinearSystem.hpp"
#include "SetupLinearSystem.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Common::XML;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;

CF::Common::ComponentBuilder < UFEM::SetupLinearSystem, Component, LibUFEM > aSetupLinearSystem_Builder;

SetupLinearSystem::SetupLinearSystem(const std::string& name) : Component ( name )
{
  this->regist_signal ( "create_model" , "Creates a linear, steady heat conduction model", "Create Model" )->signal->connect( boost::bind ( &SetupLinearSystem::signal_create_model, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  signal("create_model")->signature->connect(
      boost::bind( &SetupLinearSystem::signature_create_model, this, _1));
}

void SetupLinearSystem::signal_create_model( SignalArgs& node)
{
  SignalOptions options( node );

  // create the model
  const std::string solver_name = options.option<std::string>("Solver");
  const std::string name  = options.option<std::string>("Model name");

  CModel::Ptr model = Core::instance().root().create_component_ptr<CModelSteady>( name );

  // create the CDomain
  // CDomain::Ptr domain =
  model->create_component_ptr<CDomain>("Domain");

  // The linear system solver
  CEigenLSS::Ptr lss = model->create_component_ptr<CEigenLSS>("LSS");
  lss->mark_basic();

  // Setup solver
  CSolver::Ptr solver = build_component_abstract_type<CSolver>(solver_name, "LinearModel");
  model->add_component(solver);
  solver->mark_basic();
  solver->configure_property( "LSS", URI(lss->uri().string()) );

  // Add a VTK legacy writer by default
  CMeshWriter::Ptr mesh_writer = build_component_abstract_type<CMeshWriter>( "CF.Mesh.VTKLegacy.CWriter", "VTKLegacyWriter" );
  mesh_writer->mark_basic();
  model->add_component( mesh_writer );
}

void SetupLinearSystem::signature_create_model( SignalArgs& node )
{
  SignalOptions options( node );

  std::vector<URI> dummy;
  CFactory::Ptr linear_system_factory = Core::instance().factories().get_factory<LinearSystem>();
  std::vector<std::string> systems;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *linear_system_factory ) )
  {
    systems.push_back(bdr.name());
  }

  // create de value and add the restricted list
  options.add( "Solver", std::string() , "Available solvers", systems, " ; " );

  options.add<std::string>("Model name", std::string(), "Name for created model" );
}

} // UFEM
} // CF
