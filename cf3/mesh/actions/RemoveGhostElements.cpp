// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "RemoveGhostElements.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < RemoveGhostElements, MeshTransformer, mesh::actions::LibActions> RemoveGhostElements_Builder;

////////////////////////////////////////////////////////////////////////////////

RemoveGhostElements::RemoveGhostElements(const std::string& name) : MeshTransformer(name)
{
}

void RemoveGhostElements::execute()
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  const Uint nb_procs = comm.size();

  if(nb_procs == 1)
    return; // Boundary is always global if we only have one process

  Mesh& mesh = *m_mesh;
  MeshAdaptor adaptor(mesh);

  adaptor.prepare();
  adaptor.remove_ghost_elements();
  adaptor.finish();
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
