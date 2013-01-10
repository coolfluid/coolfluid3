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

#include "MakeBoundaryGlobal.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < MakeBoundaryGlobal, MeshTransformer, mesh::actions::LibActions> MakeBoundaryGlobal_Builder;

////////////////////////////////////////////////////////////////////////////////

MakeBoundaryGlobal::MakeBoundaryGlobal(const std::string& name) : MeshTransformer(name)
{
}

void MakeBoundaryGlobal::execute()
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  const Uint nb_procs = comm.size();
  const Uint my_rank = comm.rank();

  if(nb_procs == 1)
    return; // Boundary is always global if we only have one process

  cf3_assert(comm.is_active());

  Mesh& mesh = *m_mesh;
  MeshAdaptor adaptor(mesh);
  adaptor.prepare();

  std::vector< std::vector< std::vector<Uint> > > elements_to_send(nb_procs, std::vector< std::vector<Uint> > (m_mesh->elements().size()));
  boost_foreach(mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(mesh.topology(), IsElementsSurface()))
  {
    const Uint elements_idx = elements.entities_idx();
    const Uint nb_elems = elements.size();
    std::vector<Uint> own_elements; own_elements.reserve(nb_elems);
    for(Uint i = 0; i != nb_elems; ++i)
    {
      if(!elements.is_ghost(i))
      {
        own_elements.push_back(i);
      }
    }
    for(Uint rank = 0; rank != nb_procs; ++rank)
    {
      if(rank != my_rank)
        elements_to_send[rank][elements_idx] = own_elements;
    }

    CFdebug << "sending " << own_elements.size() << " (out of " << nb_elems << ") elements from [" << elements_idx << "] " << elements.uri().path() << " on rank " << my_rank << CFendl;
  }

  std::vector< std::vector< std::vector<Uint> > > nodes_to_send;
  adaptor.find_nodes_to_export(elements_to_send,nodes_to_send);

  std::vector< std::vector< std::vector<boost::uint64_t> > > imported_elements;
  adaptor.send_elements(elements_to_send, imported_elements);

  adaptor.flush_elements();

  std::vector< std::vector< std::vector<boost::uint64_t> > > imported_nodes;
  adaptor.send_nodes(nodes_to_send,imported_nodes);

  adaptor.flush_nodes();

  adaptor.finish();
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
