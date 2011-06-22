// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "Common/CBuilder.hpp"


#include "Mesh/Actions/LoadBalance.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshPartitioner.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LoadBalance, CMeshTransformer, LibActions> LoadBalance_Builder;

//////////////////////////////////////////////////////////////////////////////

LoadBalance::LoadBalance( const std::string& name )
: CMeshTransformer(name)
{

  properties()["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: LoadBalance Regions:array[uri]=region1,region2\n\n";
  properties()["description"] = desc;

  m_partitioner = build_component_abstract_type<CMeshPartitioner>("CF.Mesh.Zoltan.CPartitioner","partitioner");
  add_static_component(*m_partitioner);

  m_partitioner->configure_property("graph_package", std::string("PHG"));
}

/////////////////////////////////////////////////////////////////////////////

void LoadBalance::execute()
{
  CMesh& mesh = *m_mesh.lock();

  // Create global numbering and connectivity of nodes and elements (necessary for partitioning)
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_connectivity")->transform(mesh);

  // Partition the mesh
  m_partitioner->transform(mesh);

  // Create global node numbering plus ranks (Ranks are necessary for PECommPattern)
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_elem_numbering")->transform(mesh);
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
