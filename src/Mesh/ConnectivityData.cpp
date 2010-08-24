#include <boost/foreach.hpp>

#include "Common/ComponentPredicates.hpp"

#include "Mesh/ConnectivityData.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementType.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

void create_node_element_link(const Uint nb_nodes, const ElementsT& elements, const IndicesT& celements_first_elements, IndicesT& node_first_elements, CountsT& node_element_counts, IndicesT& node_elements)
{
  node_first_elements.resize(nb_nodes);
  node_element_counts.resize(nb_nodes, 0);
  
  // Count the node usage
  const Uint celement_count = elements.size();
  for(Uint celement_idx = 0; celement_idx != celement_count; ++celement_idx)
  {
    const CTable::ConnectivityTable& element_connectivity = elements[celement_idx]->connectivity_table().table();
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
  IndicesT last_added_element(nb_nodes, 0); // helper array to keep track of where we are in node_elements
  for(Uint celement_idx = 0; celement_idx != celement_count; ++celement_idx)
  {
    const CTable::ConnectivityTable& element_connectivity = elements[celement_idx]->connectivity_table().table();
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

void create_face_element_connectivity(const ElementsT& celements, const IndicesT& celements_first_elements, const IndicesT& node_first_elements, const CountsT& node_element_counts, const IndicesT& node_elements, IndicesT& celements_first_faces, BoolsT& face_has_neighbour, IndicesT& face_element_connectivity)
{
  // The first face index for each celements
  const Uint celements_count = celements.size();
  Uint sum = 0;
  celements_first_faces.resize(celements.size());
  for(Uint celement_idx = 0; celement_idx != celements_count; ++celement_idx)
  {
    celements_first_faces[celement_idx] = sum;
    sum += celements[celement_idx]->element_type().nb_faces() * celements[celement_idx]->connectivity_table().table().size();
  }
  
  // init output to the correct size
  const Uint total_face_count = celements_first_faces.back() + celements.back()->element_type().nb_faces() * celements.back()->connectivity_table().table().size();
  face_has_neighbour.resize(total_face_count, false);
  face_element_connectivity.resize(total_face_count);
  
  // fill the connectivity arrays
  for(Uint celement_idx = 0; celement_idx != celements_count; ++celement_idx)
  {
    const CTable::ConnectivityTable& element_connectivity = celements[celement_idx]->connectivity_table().table();
    const Uint elem_count = element_connectivity.size();
    const ElementType& etype = celements[celement_idx]->element_type();
    const ElementType::FaceConnectivity& face_connectivity = etype.face_connectivity();
    const Uint face_count = etype.nb_faces();
    for(Uint elem_idx = 0; elem_idx != elem_count; ++elem_idx)
    {
      const Uint global_elem_idx = celements_first_elements[celement_idx] + elem_idx;
      // loop over all faces of the element
      for(Uint face_idx = 0; face_idx != face_count; ++face_idx)
      {
        const Uint global_face_idx = celements_first_faces[celement_idx] + elem_idx * etype.nb_faces() + face_idx;
        // collect the nodes used by this face and look up the elements that use them
        IndicesT face_nodes;
        IndicesT adjacent_elements;
        adjacent_elements.reserve(32);
        BOOST_FOREACH(const Uint local_face_node, face_connectivity.face_node_range(face_idx))
        {
          // global node index
          const Uint face_node = element_connectivity[elem_idx][local_face_node];
          
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
          while(adjacent_element == adjacent_elements[i] && i != adjacent_count)
            ++i;
          if(adjacent_element != global_elem_idx)
          {
            // If any other element was found to use every node of this face, it is adjacent to the current face
            if((i - start_idx) == face_connectivity.face_node_counts[face_idx])
            {
              face_has_neighbour[global_face_idx] = true;
              face_element_connectivity[global_face_idx] = adjacent_element;
            }
          }
        }
      }
    }
  }
}

void create_face_face_connectivity(const ElementsT& celements, const IndicesT& celements_first_elements, const IndicesT& celements_first_faces, const BoolsT& face_has_neighbour, const IndicesT& face_element_connectivity, IndicesT& face_face_connectivity)
{
  face_face_connectivity.resize(face_has_neighbour.size());
  const Uint celements_count = celements.size();
  for(Uint celement_idx = 0; celement_idx != celements_count; ++celement_idx)
  {
    const Uint elements_begin = 0;
    const Uint elements_end = celements[celement_idx]->connectivity_table().table().size();
    const Uint nb_faces = celements[celement_idx]->element_type().nb_faces();
    for(Uint element_idx = elements_begin; element_idx != elements_end; ++element_idx)
    {
      const Uint global_element_idx = celements_first_elements[celement_idx] + element_idx;
      for(Uint face_idx = 0; face_idx != nb_faces; ++face_idx)
      {
        const Uint global_face_idx = celements_first_faces[celement_idx] + nb_faces * element_idx + face_idx;
        if(!face_has_neighbour[global_face_idx])
          continue;
        
        const Uint adjacent_global_elem = face_element_connectivity[global_face_idx];
        const Uint adjacent_celements = std::upper_bound(celements_first_elements.begin(), celements_first_elements.end(), adjacent_global_elem) - 1 - celements_first_elements.begin();
        const Uint adjacent_local_elem = adjacent_global_elem - celements_first_elements[adjacent_celements];
        const Uint adjacent_nb_faces = celements[adjacent_celements]->element_type().nb_faces();
        const Uint adjacent_faces_begin = celements_first_faces[adjacent_celements] + adjacent_nb_faces * adjacent_local_elem;
        const Uint adjacent_faces_end = adjacent_faces_begin + adjacent_nb_faces;
        for(Uint adjacent_face = adjacent_faces_begin; adjacent_face != adjacent_faces_end; ++adjacent_face)
        {
          if(face_has_neighbour[adjacent_face] && face_element_connectivity[adjacent_face] == global_element_idx)
          {
            face_face_connectivity[global_face_idx] = adjacent_face - adjacent_faces_begin;
          }
        }
      }
    }
  }
}



} // Mesh
} // CF
