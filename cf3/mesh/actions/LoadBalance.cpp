// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "coolfluid-packages.hpp"

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/actions/LoadBalance.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

using namespace common;
using namespace common::PE;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LoadBalance, MeshTransformer, mesh::actions::LibActions> LoadBalance_Builder;

//////////////////////////////////////////////////////////////////////////////

LoadBalance::LoadBalance( const std::string& name ) :
  MeshTransformer(name)
#if (defined CF3_HAVE_PTSCOTCH)
  ,m_partitioner(create_component("partitioner", "cf3.mesh.ptscotch.Partitioner"))
#elif (defined CF3_HAVE_ZOLTAN)
  ,m_partitioner(create_component("partitioner", "cf3.mesh.zoltan.Partitioner"))
#endif

#if ( !defined CF3_HAVE_PTSCOTCH ) && ( !defined CF3_HAVE_ZOLTAN )
#warning "There is no partitioner available (Zoltan or PT-Scotch). Load-balancing will not be possible."
#define CF3_MESH_LOADBALANCE_PARTITIONER_UNAVAILABLE
#endif
{

  properties()["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: LoadBalance Regions:array[uri]=region1,region2\n\n";
  properties()["description"] = desc;

#if (defined CF3_HAVE_PTSCOTCH)
  m_partitioner = create_component("partitioner", "cf3.mesh.ptscotch.Partitioner")->handle<MeshTransformer>();
#elif (defined CF3_HAVE_ZOLTAN)
  m_partitioner = create_component("partitioner", "cf3.mesh.zoltan.Partitioner")->handle<MeshTransformer>();
  m_partitioner->options().configure_option("graph_package", std::string("PHG"));
#endif
}

/////////////////////////////////////////////////////////////////////////////

void LoadBalance::execute()
{

  Mesh& mesh = *m_mesh;

  // balance if parallel run with multiple processors
  if( Comm::instance().is_active() && Comm::instance().size() > 1 )
  {

    CFinfo << "loadbalancing mesh:" << CFendl;

    Comm::instance().barrier();
    CFinfo << "  + building joint node & element global numbering" << CFendl;

    // build global numbering and connectivity of nodes and elements (necessary for partitioning)
    build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering")->transform(mesh);

    Comm::instance().barrier();
    CFinfo << "  + building global node-element connectivity" << CFendl;

    build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalConnectivity","glb_connectivity")->transform(mesh);

    Comm::instance().barrier();
#ifdef CF3_MESH_LOADBALANCE_PARTITIONER_UNAVAILABLE
    CFwarn << "  Skipping mesh partitioning. (No partitioner available)" << CFendl;
#else
    CFinfo << "  + partitioning and migrating" << CFendl;
    m_partitioner->transform(mesh);
#endif
    Comm::instance().barrier();
    CFinfo << "  + growing overlap layer" << CFendl;
    build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GrowOverlap","grow_overlap")->transform(mesh);

    Comm::instance().barrier();
    CFinfo << "  + deallocating unused connectivity" << CFendl;

    /// @todo check that this actually frees the memory
    //mesh.geometry_fields().glb_elem_connectivity().resize(0);

  }
  else
  {
    /// @todo disable this when below is re-enabled
    CFinfo << "  + building joint node & element global numbering" << CFendl;
    build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering")->transform(mesh);
  }


  /// @todo this has to be re-enabled
#if 0
  // Create global node indexes for nodes and elements
  // plus ranks which are necessary for CommPattern (both in serial and parallel)

  CFinfo << "creating continuous global node numbering" << CFendl;

  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumberingNodes","glb_node_numbering")->transform(mesh);

  CFinfo << "creating continuous global element numbering" << CFendl;

  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumberingElements","glb_elem_numbering")->transform(mesh);
#endif
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
