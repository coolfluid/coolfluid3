// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/Table.hpp"

#include "mesh/Space.hpp"
#include "mesh/ConnectivityData.hpp"
#include "mesh/ElementType.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

CNodeConnectivity::CNodeConnectivity(const std::string& name): Component(name)
{

}

CNodeConnectivity::~CNodeConnectivity()
{
}

boost::iterator_range<CNodeConnectivity::IndicesT::const_iterator> CNodeConnectivity::node_element_range(const Uint node_idx) const
{
  const Uint elements_begin = m_node_first_elements[node_idx];
  const Uint elements_end = elements_begin + m_node_element_counts[node_idx];
  return boost::make_iterator_range(m_node_elements.begin() + elements_begin, m_node_elements.begin() + elements_end);
}

CNodeConnectivity::ElementReferenceT CNodeConnectivity::element(const Uint global_element_idx) const
{
  const Uint celements_idx = std::upper_bound(m_celements_first_elements.begin(), m_celements_first_elements.end(), global_element_idx) - 1 - m_celements_first_elements.begin();
  return std::make_pair(m_celements_vector[celements_idx].get(), global_element_idx - m_celements_first_elements[celements_idx]) ;
}

CFaceConnectivity::CFaceConnectivity(const std::string& name): Component(name), m_node_connectivity(0)
{

}

CFaceConnectivity::~CFaceConnectivity()
{
}

void CFaceConnectivity::initialize(const Elements& own_celements, const CNodeConnectivity& node_connectivity)
{
  m_node_connectivity = &node_connectivity;
  m_element_nb_faces = own_celements.element_type().nb_faces();

  CFdebug << "Creating face connectivity for " << own_celements.uri().path() << CFendl;

  create_face_element_connectivity(own_celements,
                                   m_node_connectivity->celements_vector(),
                                   m_node_connectivity->celements_first_elements(),
                                   m_node_connectivity->node_first_elements(),
                                   m_node_connectivity->node_element_counts(),
                                   m_node_connectivity->node_elements(),
                                   m_face_has_neighbour,
                                   m_face_element_connectivity,
                                   m_face_face_connectivity
                                  );
}

void CFaceConnectivity::initialize(const CNodeConnectivity& node_connectivity)
{
  const Elements& parent_celements = *Handle<Elements>(parent());
  initialize(parent_celements, node_connectivity);
}


bool CFaceConnectivity::has_adjacent_element(const Uint element, const cf3::Uint face) const
{
  return m_face_has_neighbour[element * m_element_nb_faces + face];
}

CFaceConnectivity::ElementReferenceT CFaceConnectivity::adjacent_element(const Uint element, const Uint face) const
{
  if(!has_adjacent_element(element, face))
    throw ValueNotFound(FromHere(), "Element has no adjacent element");
  return m_node_connectivity->element(m_face_element_connectivity[element * m_element_nb_faces + face]);
}

Uint CFaceConnectivity::adjacent_face(const Uint element, const Uint face) const
{
  if(!has_adjacent_element(element, face))
    throw ValueNotFound(FromHere(), "Element has no adjacent element");
  return m_face_face_connectivity[element * m_element_nb_faces + face];
}


void create_node_element_connectivity(const Uint nb_nodes,
                                      const CFaceConnectivity::ElementsT& elements,
                                      const CFaceConnectivity::IndicesT& celements_first_elements,
                                      CFaceConnectivity::IndicesT& node_first_elements,
                                      CFaceConnectivity::CountsT& node_element_counts,
                                      CFaceConnectivity::IndicesT& node_elements)
{
  node_first_elements.resize(nb_nodes);
  node_element_counts.resize(nb_nodes, 0);

  // Count the node usage
  const Uint celement_count = elements.size();
  for(Uint celement_idx = 0; celement_idx != celement_count; ++celement_idx)
  {
    const Connectivity::ArrayT& element_connectivity = elements[celement_idx]->geometry_space().connectivity().array();
    const Uint elem_count = element_connectivity.size();
    for(Uint elem_idx = 0; elem_idx != elem_count; ++elem_idx)
    {
      BOOST_FOREACH(const Uint node_idx, element_connectivity[elem_idx])
      {
        ++node_element_counts[node_idx];
      }
    }
  }

  // Calculate the size of node_elements, and build node_first_elements
  Uint sum = 0;
  for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
  {
    node_first_elements[node_idx] = sum;
    sum += node_element_counts[node_idx];
  }

  // Build node_elements
  node_elements.resize(sum);
  CFaceConnectivity::IndicesT last_added_element(nb_nodes, 0); // helper array to keep track of where we are in node_elements
  for(Uint celement_idx = 0; celement_idx != celement_count; ++celement_idx)
  {
    const Connectivity::ArrayT& element_connectivity = elements[celement_idx]->geometry_space().connectivity().array();
    const Uint elem_count = element_connectivity.size();
    for(Uint elem_idx = 0; elem_idx != elem_count; ++elem_idx)
    {
      BOOST_FOREACH(const Uint node_idx, element_connectivity[elem_idx])
      {
        const Uint insert_idx = node_first_elements[node_idx] + last_added_element[node_idx];
        node_elements[insert_idx] = celements_first_elements[celement_idx] + elem_idx;
        ++last_added_element[node_idx];
      }
    }
  }
}

void create_face_element_connectivity(const Elements& own_celements,
                                      const CFaceConnectivity::ElementsT& celements_vector,
                                      const CFaceConnectivity::IndicesT& celements_first_elements,
                                      const CFaceConnectivity::IndicesT& node_first_elements,
                                      const CFaceConnectivity::CountsT& node_element_counts,
                                      const CFaceConnectivity::IndicesT& node_elements,
                                      CFaceConnectivity::BoolsT& face_has_neighbour,
                                      CFaceConnectivity::IndicesT& face_element_connectivity,
                                      CFaceConnectivity::IndicesT& face_face_connectivity
                                     )
{
  // Cache some commonly accessed data
  const Connectivity::ArrayT& element_connectivity = own_celements.geometry_space().connectivity().array();
  const Uint elem_count = element_connectivity.size();
  const ElementType& etype = own_celements.element_type();
  const ElementType::FaceConnectivity& face_connectivity = etype.faces();
  const Uint face_count = etype.nb_faces();
  const Uint total_face_count = face_count * elem_count;

  // Init output arrays to the correct size
  face_has_neighbour.resize(total_face_count, false);
  face_element_connectivity.resize(total_face_count);
  face_face_connectivity.resize(total_face_count);

  // Find out if own_celements is part of celements_vector
  Uint global_offset = 0;
  bool vector_has_own_celements = false;
  for(Uint i = 0; i != celements_vector.size(); ++i)
  {
    if(&own_celements == celements_vector[i].get())
    {
      vector_has_own_celements = true;
      global_offset = celements_first_elements[i];
      break;
    }
  }

  // fill the output arrays
  for(Uint elem_idx = 0; elem_idx != elem_count; ++elem_idx)
  {
    // loop over all faces of the element
    for(Uint face_idx = 0; face_idx != face_count; ++face_idx)
    {
      // collect the nodes used by this face and look up the elements that use them
      CFaceConnectivity::IndicesT face_nodes; face_nodes.reserve(face_connectivity.stride[face_idx]);
      CFaceConnectivity::IndicesT adjacent_elements;
      adjacent_elements.reserve(32); // 32 in the case of a structured hexahedral mesh, just to avoid too many allocations in common cases
      BOOST_FOREACH(const Uint local_face_node, face_connectivity.nodes_range(face_idx))
      {
        // global node index
        const Uint face_node = element_connectivity[elem_idx][local_face_node];
        face_nodes.push_back(face_node);

        // collect other elements that use this node
        const Uint node_elements_begin = node_first_elements[face_node];
        const Uint node_elements_end = node_element_counts[face_node] + node_elements_begin;
        adjacent_elements.insert(adjacent_elements.end(), node_elements.begin() + node_elements_begin, node_elements.begin() + node_elements_end);
      }

      // Find the adjacent element, if any
      std::sort(adjacent_elements.begin(), adjacent_elements.end());
      const Uint adjacent_count = adjacent_elements.size();
      for(Uint i = 0; i != adjacent_count;)
      {
        const Uint adjacent_element = adjacent_elements[i];
        const Uint start_idx = i;
        while(i != adjacent_count && adjacent_element == adjacent_elements[i])
          ++i;
        if(!vector_has_own_celements || (vector_has_own_celements && adjacent_element != (global_offset + elem_idx)))
        {
          // If any other element was found to use every node of this face, it is adjacent to the current face
          if((i - start_idx) == face_connectivity.stride[face_idx])
          {
            // Verify that the face orientation is opposite to our own
            const Uint adjacent_celements_idx = std::upper_bound(celements_first_elements.begin(), celements_first_elements.end(), adjacent_element) - 1 - celements_first_elements.begin();
            const Elements& adjacent_celements = *celements_vector[adjacent_celements_idx];
            const Uint adjacent_local_elem = adjacent_element - celements_first_elements[adjacent_celements_idx];
            const Connectivity::ArrayT& adjacent_connectivity_table = adjacent_celements.geometry_space().connectivity().array();
            const Uint adjacent_nb_faces = adjacent_celements.element_type().nb_faces();
            const ElementType::FaceConnectivity& faces = adjacent_celements.element_type().faces();
            for(Uint adj_face_idx = 0; adj_face_idx != adjacent_nb_faces; ++adj_face_idx)
            {
              CFaceConnectivity::IndicesT adjacent_face_nodes;
              adjacent_face_nodes.reserve(faces.stride[adj_face_idx]);
              BOOST_FOREACH(const Uint local_node, faces.nodes_range(adj_face_idx))
              {
                adjacent_face_nodes.push_back(adjacent_connectivity_table[adjacent_local_elem][local_node]);
              }

              // A face will match if its node ordering is reversed relative to our own for internal faces, or if the ordering is the same on boundaries
              if(own_celements.element_type().dimensionality() == adjacent_celements.element_type().dimensionality())
                std::reverse(adjacent_face_nodes.begin(), adjacent_face_nodes.end());
              // We need to start at the same node, except for line segments
              if(adjacent_face_nodes.size() > 2)
              {
                // Try to find the first node of the face in the candidate-adjacent-face
                const CFaceConnectivity::IndicesT::iterator first_matching_node = std::find(adjacent_face_nodes.begin(), adjacent_face_nodes.end(), face_nodes.front());
                // Roll the node list of the candidate match to start at the same node
                std::rotate(adjacent_face_nodes.begin(), first_matching_node, adjacent_face_nodes.end());
              }

              if(adjacent_face_nodes == face_nodes) // now we have a match if the node lists are identical
              {
                const Uint global_face_idx = elem_idx * face_count + face_idx;
                cf3_assert(!face_has_neighbour[global_face_idx]); // We must find only one matching face
                face_has_neighbour[global_face_idx] = true;
                face_element_connectivity[global_face_idx] = adjacent_element;
                face_face_connectivity[global_face_idx] = adj_face_idx;
              }
            }
          }
        }
      }
    }
  }
}

/// Gets a sorted list of face nodes
void sorted_face_nodes(const Elements& celements, const common::Table<Uint>::ArrayT& connectivity_table, const Uint element_idx, const Uint face_idx, CFaceConnectivity::IndicesT& face_nodes)
{
  face_nodes.reserve(celements.element_type().faces().stride[face_idx]);
  BOOST_FOREACH(const Uint local_node, celements.element_type().faces().nodes_range(face_idx))
  {
    face_nodes.push_back(connectivity_table[element_idx][local_node]);
  }
  std::sort(face_nodes.begin(), face_nodes.end());
}

void create_face_face_connectivity(const Elements& own_celements, const CFaceConnectivity::ElementsT& celements_vector, const CFaceConnectivity::IndicesT& celements_first_elements, const CFaceConnectivity::BoolsT& face_has_neighbour, const CFaceConnectivity::IndicesT& face_element_connectivity, CFaceConnectivity::IndicesT& face_face_connectivity)
{
  // Init to correct size
  face_face_connectivity.resize(face_has_neighbour.size());

  // Find out if own_celements is part of celements_vector
  Uint global_offset = 0;
  bool vector_has_own_celements = false;
  Uint own_celements_idx = 0;
  for(Uint i = 0; i != celements_vector.size(); ++i)
  {
    if(&own_celements == celements_vector[i].get())
    {
      vector_has_own_celements = true;
      global_offset = celements_first_elements[i];
      own_celements_idx = i;
      break;
    }
  }

  const Connectivity::ArrayT& connectivity_table = own_celements.geometry_space().connectivity().array();
  const Uint elements_begin = 0;
  const Uint elements_end = connectivity_table.size();
  const Uint nb_faces = own_celements.element_type().nb_faces();
  for(Uint element_idx = elements_begin; element_idx != elements_end; ++element_idx)
  {
    //const Uint global_element_idx = global_offset + element_idx;
    for(Uint face_idx = 0; face_idx != nb_faces; ++face_idx)
    {
      const Uint global_face_idx = nb_faces * element_idx + face_idx;
      if(!face_has_neighbour[global_face_idx])
        continue;

      // find out what our adjacent element is
      const Uint adjacent_global_elem = face_element_connectivity[global_face_idx];
      const Uint adjacent_celements_idx = std::upper_bound(celements_first_elements.begin(), celements_first_elements.end(), adjacent_global_elem) - 1 - celements_first_elements.begin();
      const Elements& adjacent_celements = *celements_vector[adjacent_celements_idx];
      const Uint adjacent_nb_faces = adjacent_celements.element_type().nb_faces();
      // if we have a neighbour inside own_celements, we can use the face_element_connectivity table for fast lookup of the connecting face
      if(vector_has_own_celements && adjacent_celements_idx == own_celements_idx)
      {
        const Uint adjacent_local_elem = adjacent_global_elem - global_offset;
        const Uint adjacent_faces_begin = adjacent_local_elem * adjacent_nb_faces;
        const Uint adjacent_faces_end = adjacent_faces_begin + adjacent_nb_faces;
        for(Uint adjacent_face = adjacent_faces_begin; adjacent_face != adjacent_faces_end; ++adjacent_face)
        {
          if(face_has_neighbour[adjacent_face] && face_element_connectivity[adjacent_face] == element_idx)
          {
            face_face_connectivity[global_face_idx] = adjacent_face - adjacent_faces_begin;
            break;
          }
        }
      }
      else // otherwise we need to compare face node indices, which is slower
      {
        CFaceConnectivity::IndicesT face_nodes;
        sorted_face_nodes(own_celements, connectivity_table, element_idx, face_idx, face_nodes);
        const Uint adjacent_local_elem = adjacent_global_elem - celements_first_elements[adjacent_celements_idx];
        const Connectivity::ArrayT& adjacent_connectivity_table = adjacent_celements.geometry_space().connectivity().array();
        for(Uint adjacent_face = 0; adjacent_face != adjacent_nb_faces; ++adjacent_face)
        {
          CFaceConnectivity::IndicesT adjacent_face_nodes;
          sorted_face_nodes(adjacent_celements, adjacent_connectivity_table, adjacent_local_elem, adjacent_face, adjacent_face_nodes);

          if(face_nodes == adjacent_face_nodes)
          {
            face_face_connectivity[global_face_idx] = adjacent_face;
            break;
          }
        }
      }
    }
  }

}



} // mesh
} // cf3
