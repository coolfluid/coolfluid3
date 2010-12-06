// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
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

Common::ComponentBuilder < LoadMesh, Component, LibSolver > LoadMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

LoadMesh::LoadMesh ( const std::string& name  ) :
  Component ( name )
{

  //  add_component ( create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" ) );

  // options
  //  m_properties.add_option< OptionT<std::string> >  ( "Model",  "Model to fill, if empty a new model will be created in the root" , "" );
  //  m_properties["Model"].as_option().mark_basic();

  // signals

  regist_signal ( "load_mesh" , "Loads meshes, guessing automatically the format", "Load meshes" )->connect ( boost::bind ( &LoadMesh::load_mesh, this, _1 ) );

  signal("load_mesh").signature
      .insert<URI>("Path to domain", "Path to the domain to hold the mesh");

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;
}

////////////////////////////////////////////////////////////////////////////////

LoadMesh::~LoadMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::load_mesh ( Common::XmlNode& node )
{
   XmlParams params (node);
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
