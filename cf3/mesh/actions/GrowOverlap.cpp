// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>
#include <map>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"


#include "common/PE/Comm.hpp"
#include "common/PE/Buffer.hpp"
#include "common/PE/debug.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/Space.hpp"

#include "mesh/actions/GrowOverlap.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  using namespace common::PE;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < GrowOverlap, MeshTransformer, mesh::actions::LibActions> GrowOverlap_Builder;

////////////////////////////////////////////////////////////////////////////////

GrowOverlap::GrowOverlap( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Grows the overlap layer of the mesh");
  std::string desc;
  desc =
      " Boundary nodes of one rank are communicated to other ranks.\n"
      " Each other rank then communicates all elements that are connected \n"
      " to these boundary nodes. \n"
      " Missing nodes are then also communicated to complete the elements";
  properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

void GrowOverlap::execute()
{
  // These 2 functions need to be called
  m_mesh->geometry_fields().rebuild_map_glb_to_loc();
  m_mesh->geometry_fields().rebuild_node_to_element_connectivity();

  MeshAdaptor mesh_adaptor(*m_mesh);
  mesh_adaptor.prepare();
  mesh_adaptor.grow_overlap();
  mesh_adaptor.finish();

}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
