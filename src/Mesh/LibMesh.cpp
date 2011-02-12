// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/LoadMesh.hpp"
#include "Mesh/WriteMesh.hpp"

namespace CF {
namespace Mesh {

using namespace CF::Common;

CF::Common::RegistLibrary<LibMesh> libMesh;

////////////////////////////////////////////////////////////////////////////////

void LibMesh::initiate()
{
  Core::instance().root()
      ->get_child("Tools")
      ->create_component<Mesh::LoadMesh>( "LoadMesh" )
      ->mark_basic();
      
  Core::instance().root()
      ->get_child("Tools")
      ->create_component<Mesh::WriteMesh>( "WriteMesh" )
      ->mark_basic();
}

void LibMesh::terminate()
{
  Core::instance().root()
      ->get_child("Tools")
      ->remove_component("LoadMesh");
  Core::instance().root()
      ->get_child("Tools")
      ->remove_component("WriteMesh");

}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
