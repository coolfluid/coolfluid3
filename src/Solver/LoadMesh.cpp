// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CRegion.hpp"

#include "Mesh/Neu/CReader.hpp"

#include "Solver/LoadMesh.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::String;
using namespace CF::Mesh;

Common::ComponentBuilder < LoadMesh, Component, LibSolver > LoadMesh_Builder ( LoadMesh::type_name() );

////////////////////////////////////////////////////////////////////////////////

LoadMesh::LoadMesh ( const std::string& name  ) :
  Component ( name )
{
  BuildComponent<full>().build(this);

//  add_component ( create_component_abstract_type<CMeshReader>( "Neu", "NeutralReader" ) );

}

////////////////////////////////////////////////////////////////////////////////

LoadMesh::~LoadMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::define_config_properties(Common::PropertyList& options)
{
//  options.add_option< OptionT<std::string> >  ( "Model",  "Model to fill, if empty a new model will be created in the root" , "" );
//  options["Model"].as_option().mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::regist_signals ( LoadMesh* self )
{
  this->regist_signal ( "run_wizard" , "runs the wizard ", "Run Wizard" )->connect ( boost::bind ( &LoadMesh::run_wizard, self, _1 ) );

  this->signal("run_wizard").m_signature.insert<std::string>("mesh name", "name for created mesh component")
                                        .insert<URI>("path to domain", "path to the domain to hold the mesh");

}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::run_wizard ( Common::XmlNode& node )
{

   XmlParams params (node);

   CFinfo << params.get_option<std::string>("mesh name") << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
