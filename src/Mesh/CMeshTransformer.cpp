// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ComponentPredicates.hpp"
#include "Common/CLink.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/StringConversion.hpp"

#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::CMeshTransformer ( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();

  OptionURI::Ptr option;
  option = boost::dynamic_pointer_cast<OptionURI>(m_properties.add_option<OptionURI>("Mesh","The mesh to be transformed",URI()));
  option->supported_protocol(CF::Common::URI::Scheme::CPATH);
  option->attach_trigger ( boost::bind ( &CMeshTransformer::config_mesh,   this ) );
  option->mark_basic();

  m_mesh_link = create_static_component<CLink>("mesh");

}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::config_mesh()
{
  URI mesh_uri;
  property("Mesh").put_value(mesh_uri);
  CMesh::Ptr mesh = Core::instance().root()->look_component<CMesh>(mesh_uri);
  if ( is_null(mesh) )
    throw CastingFailed (FromHere(), "Mesh must be of a CMesh type");
  set_mesh(mesh);
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::set_mesh(CMesh::Ptr mesh)
{
  m_mesh_link->link_to(mesh);
}

////////////////////////////////////////////////////////////////////////////////

CMeshTransformer::~CMeshTransformer()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshTransformer::execute()
{
  std::vector<std::string> args;
  if (is_null(m_mesh_link->follow()))
    throw BadPointer(FromHere(),"Mesh option is not set");
  if (is_null(m_mesh_link->follow()->as_type<CMesh>()))
    throw CastingFailed (FromHere(), "Mesh must be of a CMesh type");
  transform(m_mesh_link->follow()->as_type<CMesh>());
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
