// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
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

Common::ObjectProvider < LoadMesh, Component, LibSolver, NB_ARGS_1 >
LoadMesh_Provider ( LoadMesh::type_name() );

////////////////////////////////////////////////////////////////////////////////

LoadMesh::LoadMesh ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;

//  add_component ( create_component_abstract_type<CMeshReader>( "Neu", "NeutralReader" ) );

}

////////////////////////////////////////////////////////////////////////////////

LoadMesh::~LoadMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::defineConfigProperties(Common::PropertyList& options)
{
//  options.add_option< OptionT<std::string> >  ( "Model",  "Model to fill, if empty a new model will be created in the root" , "" );
//  options["Model"].as_option().mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::regist_signals ( LoadMesh* self )
{
  self->regist_signal ( "run_wizard" , "runs the wizard ", "Run Wizard" )->connect ( boost::bind ( &LoadMesh::run_wizard, self, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::run_wizard ( Common::XmlNode& node )
{



}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
