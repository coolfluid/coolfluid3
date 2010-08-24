#ifndef CF_Mesh_ConnectivityData_hpp
#define CF_Mesh_ConnectivityData_hpp

#include <boost/shared_ptr.hpp>

#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"

////////////////////////////////////////////////////////////////////////////////


namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

/// Storage for a list of elements
typedef std::vector<CElements::ConstPtr> ElementsT;

/// Storage for counted numbers
typedef std::vector<Uint> CountsT;

/// Storage for indices into other arrays
typedef std::vector<Uint> IndicesT;

/// Storage for a list of boolean values
typedef std::vector<bool> BoolsT;

/// Create a vector containing all the CElements in the given range
/// @param [out] celements_vector the CElements contained in range
/// @param [out] celements_first_elements the first index into a global element array if all CElements would be concatenated
template <typename RangeT>
void create_celements_vector(const RangeT& range, ElementsT& celements_vector, IndicesT& celements_first_elements)
{
  celements_vector.clear();

  for(typename RangeT::const_iterator elem = range.begin(); elem != range.end(); ++elem)
  {
    celements_vector.push_back(elem.base().get());
  }
  
  /// Construct global index starts
  celements_first_elements.clear();
  Uint sum = 0;
  BOOST_FOREACH(const CElements& elements, range)
  {
    celements_first_elements.push_back(sum);
    sum += elements.connectivity_table().table().size();
  }
}

/// Store connectivity from a node to the elements that use it
/// @param [in] nb_nodes The total number of nodes to consider. This should be equal to the size of the coordinate table.
/// @param [in] elements The vector with the CElements to consider. The index of this vector is used to uniquely identify an element
/// @param [out] node_first_elements For each node, the indices of the first element in node_elements. Size equal to nb_nodes.
/// @param [out] node_element_counts For each node, the number of elements that use it. Size equal to nb_nodes
/// @param [out] node_elements The link between a node and the elements that use it. The columns contain the index into the elements
/// vector and then the index into that CElements connectivity table. Size is the sum of the elements in node_element_counts.
/// The elements using node i are located from node_elements[node_first_elements[i]] to node_elements[node_first_elements[i] + node_element_counts[i]].
void create_node_element_link( const Uint nb_nodes, const Mesh::ElementsT& elements, const IndicesT& celements_first_elements, IndicesT& node_first_elements, CountsT& node_element_counts, IndicesT& node_elements);

/// Calculate the face connectivity data, based on inputs as calculated by create_node_element_link.
/// @param [out] celement_first_faces For each celements, the index of its first face in the global array
/// @param [out] face_has_neighbour For each face of each global element, true if the face has a neighbour element
/// @param [out] face_element_connectivity For each face of each global element, store the element it is connected to. The length of this vector is the total number of elements from
/// all the CElements in elements, multiplied with the number of faces for each element. To get the connectivity of face 3 of element j of CElements i, use:
/// face_element_connectivity[celement_first_face[i] + nb_faces*j + 3]
void create_face_element_connectivity( const ElementsT& celements, const IndicesT& celements_first_elements, const IndicesT& node_first_elements, const CountsT& node_element_counts, const IndicesT& node_elements, IndicesT& celements_first_faces, BoolsT& face_has_neighbour, IndicesT& face_element_connectivity);

/// Calculate the local face index in adjacent elements
void create_face_face_connectivity( const ElementsT& celements, const IndicesT& celements_first_elements, const IndicesT& celements_first_faces, const BoolsT& face_has_neighbour, const IndicesT& face_element_connectivity, IndicesT& face_face_connectivity);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ConnectivityData_hpp
