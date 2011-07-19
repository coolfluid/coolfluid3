// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"

#include "Common/MPI/PE.hpp"

#include "Mesh/Actions/LoadBalance.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshPartitioner.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

using namespace Common;
using namespace Common::mpi;

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

  m_partitioner->configure_option("graph_package", std::string("PHG"));
}

/////////////////////////////////////////////////////////////////////////////

void LoadBalance::execute()
{

  CMesh& mesh = *m_mesh.lock();

  // balance if parallel run with multiple processors
  if( PE::instance().is_active() && PE::instance().size() > 1 )
  {

    CFinfo << "loadbalancing mesh:" << CFendl;

    CFinfo << "  + building joint node & element global numbering" << CFendl;

    // build global numbering and connectivity of nodes and elements (necessary for partitioning)
    build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering")->transform(mesh);

    CFinfo << "  + building global node-element connectivity" << CFendl;

    build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_connectivity")->transform(mesh);


    CFinfo << "  + partitioning and migrating" << CFendl;
    m_partitioner->transform(mesh);

    CFinfo << "  + growing overlap layer" << CFendl;
    build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.GrowOverlap","grow_overlap")->transform(mesh);


    CFinfo << "  + deallocating unused connectivity" << CFendl;

    /// @todo check that this actually frees the memory
    //mesh.nodes().glb_elem_connectivity().resize(0);

  }
  else
  {
    /// @todo disable this when below is re-enabled
    CFinfo << "  + building joint node & element global numbering" << CFendl;
    build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering")->transform(mesh);
  }


  /// @todo this has to be re-enabled
#if 0
  // Create global node indexes for nodes and elements
  // plus ranks which are necessary for PECommPattern (both in serial and parallel)

  CFinfo << "creating continuous global node numbering" << CFendl;

  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);

  CFinfo << "creating continuous global element numbering" << CFendl;

  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_elem_numbering")->transform(mesh);
#endif
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
