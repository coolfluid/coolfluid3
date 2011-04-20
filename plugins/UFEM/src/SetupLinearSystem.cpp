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
#include "Common/CreateComponent.hpp"

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
  this->regist_signal ( "create_model" , "Creates a linear, steady heat conduction model", "Create Model" )->signal->connect( boost::bind ( &SetupLinearSystem::create_model, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  signal("create_model")->signature->connect(
      boost::bind( &SetupLinearSystem::create_model_signature, this, _1));
}

void SetupLinearSystem::create_model( SignalArgs& node)
{
  SignalOptions options( node );

  // create the model
  const std::string solver_name = options.option<std::string>("Solver");
  const std::string name  = options.option<std::string>("Model name");

  CModel::Ptr model = Core::instance().root().create_component<CModelSteady>( name );

  // create the CDomain
  // CDomain::Ptr domain =
  model->create_component<CDomain>("Domain");

  // The linear system solver
  CEigenLSS::Ptr lss = model->create_component<CEigenLSS>("LSS");
  lss->mark_basic();

  // Setup method
  LinearSystem::Ptr hc = create_component_abstract_type<LinearSystem>(solver_name, "LinearModel");
  model->add_component(hc);
  hc->mark_basic();
  hc->configure_property( "LSS", URI(lss->full_path().string()) );

  CMeshWriter::Ptr mesh_writer = create_component_abstract_type<CMeshWriter>( "CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  mesh_writer->mark_basic();
  model->add_component( mesh_writer );
  
  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" );
  mesh_reader->mark_basic();
  model->add_component( mesh_reader );
}

void SetupLinearSystem::create_model_signature( SignalArgs& node )
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
