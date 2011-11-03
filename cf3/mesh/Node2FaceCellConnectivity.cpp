// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/FindComponents.hpp"

#include "common/Link.hpp"
#include "common/Builder.hpp"
#include "mesh/Node2FaceCellConnectivity.hpp"
#include "common/DynTable.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Region.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < Node2FaceCellConnectivity , Component, LibMesh > Node2FaceCellConnectivity_Builder;

////////////////////////////////////////////////////////////////////////////////

Node2FaceCellConnectivity::Node2FaceCellConnectivity ( const std::string& name ) :
  Component(name)
{
  m_used_components = create_static_component_ptr<Group>("used_components");

  m_nodes = create_static_component_ptr<common::Link>(mesh::Tags::nodes());
  m_connectivity = create_static_component_ptr<DynTable<Face2Cell> >(mesh::Tags::connectivity_table());
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void Node2FaceCellConnectivity::setup(Region& region)
{
  boost_foreach( FaceCellConnectivity& faces,  find_components_recursively<FaceCellConnectivity>(region) )
    add_used(faces);

  build_connectivity();
}

////////////////////////////////////////////////////////////////////////////////

std::vector<FaceCellConnectivity::Ptr> Node2FaceCellConnectivity::used()
{
  std::vector<FaceCellConnectivity::Ptr> vec;
  boost_foreach( Link& link, find_components<Link>(*m_used_components) )
  {
    vec.push_back(link.follow()->as_ptr<FaceCellConnectivity>());
  }
  return vec;
}

////////////////////////////////////////////////////////////////////////////////

void Node2FaceCellConnectivity::add_used (const FaceCellConnectivity& used_comp)
{
  bool found = false;
  std::vector<FaceCellConnectivity::Ptr> used_components = used();
  boost_foreach( FaceCellConnectivity::Ptr comp, used_components )
  {
    if (comp->as_ptr<Component>() == used_comp.follow())
    {
      found = true;
      break;
    }
  }
  if (found == false)
    m_used_components->create_component_ptr<Link>("used_component["+to_str(used_components.size())+"]")->link_to(used_comp);

}

////////////////////////////////////////////////////////////////////////////////

void Node2FaceCellConnectivity::set_nodes(SpaceFields& nodes)
{
  m_nodes->link_to(nodes.self());
  m_connectivity->resize(nodes.size());
}

////////////////////////////////////////////////////////////////////////////////

void Node2FaceCellConnectivity::build_connectivity()
{
  SpaceFields const& nodes = *m_nodes->follow()->as_ptr<SpaceFields>();

  // Reserve memory in m_connectivity->array()
  std::vector<Uint> connectivity_sizes(nodes.size());
  boost_foreach(FaceCellConnectivity::Ptr face_cell_connectivity_comp, used() )
  {
    FaceCellConnectivity& face_cell_connectivity = *face_cell_connectivity_comp;

    for (Face2Cell face(face_cell_connectivity); face.idx<face_cell_connectivity.size(); ++face.idx)
    {
      if (face.is_bdry())
      {
        boost_foreach (const Uint node_idx, face.nodes())
        {
          ++connectivity_sizes[node_idx];
        }

      }
    }
  }
  Uint i(0);
  boost_foreach (DynTable<Face2Cell>::Row row, m_connectivity->array() )
  {
    row.reserve(connectivity_sizes[i++]);
  }

  // fill m_connectivity->array()
  boost_foreach(FaceCellConnectivity::Ptr face_cell_connectivity_comp, used() )
  {
    FaceCellConnectivity& face_cell_connectivity = *face_cell_connectivity_comp;

    for (Face2Cell face(face_cell_connectivity); face.idx<face_cell_connectivity.size(); ++face.idx)
    {
      if (face.is_bdry())
      {
        boost_foreach (const Uint node_idx, face.nodes())
        {
          m_connectivity->array()[node_idx].push_back(face);
        }
      }
    }
  }

//  Uint node=0;
//  boost_foreach(DynTable<Face2Cell>::ConstRow faces, m_connectivity->array())
//  {
//    std::cout << node++ << "  : " << std::endl;
//    boost_foreach(Face2Cell face, faces)
//        std::cout << "   - face " << face.idx<< " --> " << face.cells()[0] <<std::endl;
//  }

}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3