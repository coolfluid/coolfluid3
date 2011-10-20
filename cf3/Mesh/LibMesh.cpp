// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/RegistLibrary.hpp"
#include "common/CRoot.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/LoadMesh.hpp"
#include "Mesh/WriteMesh.hpp"

namespace cf3 {
namespace Mesh {

using namespace cf3::common;

cf3::common::RegistLibrary<LibMesh> libMesh;

////////////////////////////////////////////////////////////////////////////////

void LibMesh::initiate_impl()
{
  Core::instance().tools().create_component_ptr<Mesh::LoadMesh>( "LoadMesh" )
      ->mark_basic();

  Core::instance().tools().create_component_ptr<Mesh::WriteMesh>( "WriteMesh" )
      ->mark_basic();
}

void LibMesh::terminate_impl()
{
  Core::instance().tools().remove_component("LoadMesh");
  Core::instance().tools().remove_component("WriteMesh");
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3
