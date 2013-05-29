// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"
#include "common/Group.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/LoadMesh.hpp"
#include "mesh/WriteMesh.hpp"

namespace cf3 {
namespace mesh {

using namespace cf3::common;

cf3::common::RegistLibrary<LibMesh> libMesh;

////////////////////////////////////////////////////////////////////////////////

void LibMesh::initiate()
{
  if(m_is_initiated)
    return;

  initiate_impl();
  m_is_initiated = true;
}


void LibMesh::terminate()
{
  if(!m_is_initiated)
    return;

  terminate_impl();
  m_is_initiated = false;
}


void LibMesh::initiate_impl()
{
  m_load_mesh = Core::instance().tools().create_component<mesh::LoadMesh>( "LoadMesh" );
  m_load_mesh->mark_basic();

  m_write_mesh =  Core::instance().tools().create_component<mesh::WriteMesh>( "WriteMesh" );
  m_write_mesh->mark_basic();
}

void LibMesh::terminate_impl()
{
  if(is_not_null(m_load_mesh))
    Core::instance().tools().remove_component("LoadMesh");

  if(is_not_null(m_write_mesh))
    Core::instance().tools().remove_component("WriteMesh");
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
