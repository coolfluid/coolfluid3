// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Entities.hpp"
#include "mesh/MeshElements.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;

ComponentBuilder<MeshElements, Component, LibMesh> MeshElements_Builder;

////////////////////////////////////////////////////////////////////////////////

MeshElements::MeshElements(const std::string& name) :
  UnifiedData(name)
{
}

////////////////////////////////////////////////////////////////////////////////

void MeshElements::update()
{
  Mesh& mesh = find_parent_component<Mesh>(*this);
  boost_foreach(Entities& elements, find_components_recursively<Entities>(mesh.topology()))
    add(elements);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
