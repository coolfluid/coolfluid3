// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ConnectivityData_hpp
#define cf3_mesh_ConnectivityData_hpp

#include <set>

#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Connectivity.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between nodes and their adjacent elements
/// and provides a convenient API to access the data
class Mesh_API NodeConnectivity : public common::Component
{
public:
  /// Storage for a list of Entities components
  typedef std::vector<Handle< Entities const> > EntitiesT;

  /// Storage for counted numbers
  typedef std::vector<Uint> CountsT;

  /// Storage for indices into other arrays
  typedef std::vector<Uint> IndicesT;

  /// Uniquely refer an element by its index into the list of entities and it's local element index
  typedef std::pair<Uint, Uint> ElementReferenceT;
  
  /// Type of the container used to store the actual connectivity
  typedef std::vector<ElementReferenceT> NodeElementsT;

  /// Contructor
  /// @param name of the component
  NodeConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~NodeConnectivity();

  /// Get the class name
  static std::string type_name () { return "NodeConnectivity"; }

  // functions specific to the NodeConnectivity component

  /// Initialize the connectivity arrays, based on a list of Entities
  /// @see create_node_element_connectivity
  void initialize(const EntitiesT& entities);

  /// Allow explicitely supplying the number of nodes
  void initialize(const Uint nb_nodes, const EntitiesT& entities);

  /// Overloads for non-const entities as returned by Mesh::entities()
  void initialize(const std::vector< Handle<Entities> >& entities);
  void initialize(const Uint nb_nodes, const std::vector< Handle<Entities> >& entities);

  /// Initialization using a range
  template<typename RangeT>
  void initialize(const RangeT& range)
  {
    EntitiesT entities;
    entities.reserve(range.size());
    BOOST_FOREACH(const Entities& ent, range)
    {
      entities.push_back(ent.handle<Entities const>());
    }

    initialize(entities);
  }

  /// Range of global element indices that use the node with index node_idx.
  boost::iterator_range<NodeElementsT::const_iterator> node_element_range(const Uint node_idx) const;

  /// Access to the raw data
  const EntitiesT& entities() const { return m_entities; }  
  const IndicesT& node_first_elements() const { return m_node_first_elements; }
  const CountsT& node_element_counts() const { return m_node_element_counts; }
  const NodeElementsT& node_elements() const { return m_node_elements; }

private: // data
  EntitiesT m_entities;
  IndicesT m_node_first_elements;
  CountsT m_node_element_counts;
  NodeElementsT m_node_elements;
}; // NodeConnectivity

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between element faces and their adjacent elements
/// and provides a convenient API to access the data
class Mesh_API FaceConnectivity : public common::Component
{
public:
  /// Storage for a list of elements
  typedef NodeConnectivity::EntitiesT EntitiesT;

  /// Storage for counted numbers
  typedef NodeConnectivity::CountsT CountsT;

  /// Storage for indices into other arrays
  typedef NodeConnectivity::IndicesT IndicesT;

  /// Storage for a list of boolean values
  typedef std::vector<bool> BoolsT;

  /// Uniquely refer an element by its Elements and local element index
  typedef NodeConnectivity::ElementReferenceT ElementReferenceT;

  /// Contructor
  /// @param name of the component
  FaceConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~FaceConnectivity();

  /// Get the class name
  static std::string type_name () { return "FaceConnectivity"; }

  // functions specific to the FaceConnectivity component

  /// Initialize the connectivity arrays so the adjacent face- and element lookups are defined for celements
  /// Parameters use the convention from the low level interface
  /// @see create_node_element_connectivity
  /// @see create_face_element_connectivity
  /// @see create_face_face_connectivity
  void initialize(const Elements& own_celements, const NodeConnectivity& node_connectivity);

  /// Shortcut assuming the parent is the own_celements
  void initialize(const NodeConnectivity& node_connectivity);

  /// True if the given face of the given element has an adjacent element
  bool has_adjacent_element(const Uint element, const Uint face) const;

  /// Get the element that is adjacent to the given face of the given element
  ElementReferenceT adjacent_element(const Uint element, const Uint face) const;

  /// Get the face that is adjacent to the given face of the given element
  Uint adjacent_face(const Uint element, const Uint face) const;


  /// Access to the raw data
  Uint element_nb_faces() const { return m_element_nb_faces; }
  const BoolsT& face_has_neighbour() const { return m_face_has_neighbour; }
  const NodeConnectivity::NodeElementsT& face_element_connectivity() const { return m_face_element_connectivity; }
  const IndicesT& face_face_connectivity() const { return m_face_face_connectivity; }
  const NodeConnectivity& node_connectivity() const { return *m_node_connectivity; }

private: // data
  const NodeConnectivity* m_node_connectivity; // normal pointer for performance reasons

  /// Number of faces per element
  Uint m_element_nb_faces;

  BoolsT m_face_has_neighbour;
  NodeConnectivity::NodeElementsT m_face_element_connectivity;
  IndicesT m_face_face_connectivity;
}; // FaceConnectivity

////////////////////////////////////////////////////////////////////////////////

/// Store connectivity from a node to the elements that use it
/// @param [in] nb_nodes The total number of nodes to consider. This should be equal to the size of the coordinate table.
/// @param [in] entities The vector with the Entities to consider. The index of this vector is used to uniquely identify an element
/// @param [out] node_first_elements For each node, the indices of the first element in node_elements. Size equal to nb_nodes.
/// @param [out] node_element_counts For each node, the number of elements that use it. Size equal to nb_nodes
/// @param [out] node_elements The link between a node and the elements that use it. The columns contain the index into the elements
/// vector and then the index into that Elements connectivity table. Size is the sum of the elements in node_element_counts.
/// The elements using node i are located from node_elements[node_first_elements[i]] to node_elements[node_first_elements[i] + node_element_counts[i]].
void Mesh_API create_node_element_connectivity( const Uint nb_nodes, const NodeConnectivity::EntitiesT& entities, NodeConnectivity::IndicesT& node_first_elements, NodeConnectivity::CountsT& node_element_counts, NodeConnectivity::NodeElementsT& node_elements);

/// Calculate the face connectivity data, based on inputs as calculated by create_node_element_connectivity.
/// @param [in] own_celements The Elements for which the connectivity data is needed
/// @param [in] entities
/// @param [in] node_first_elements
/// @param [in] node_element_counts
/// @param [in] node_elements
/// @param [out] face_has_neighbour For each face of each global element, true if the face has a neighbour element
/// @param [out] face_element_connectivity For each face of element in own_celements, store the element it is connected to. The length of this vector is the total number of elements from
/// own_celements, multiplied with the number of faces for each element. To get the connectivity of face 3 of element i of own_celements, use:
/// face_element_connectivity[nb_faces*i + 3]
/// @param [out] face_face_connectivity Stores to local face index for each global face index
void Mesh_API create_face_element_connectivity( const Entities& own_celements, const NodeConnectivity::EntitiesT& entities, const NodeConnectivity::IndicesT& node_first_elements, const NodeConnectivity::CountsT& node_element_counts, const NodeConnectivity::NodeElementsT& node_elements, FaceConnectivity::BoolsT& face_has_neighbour, NodeConnectivity::NodeElementsT& face_element_connectivity, NodeConnectivity::IndicesT& face_face_connectivity);

/// Calculate the local face index in adjacent elements
/// @param [in]  own_celements               The Elements for which the connectivity data is needed
/// @param [in]  entities
/// @param [in]  face_has_neighbour          For each face of each global element, true if the face has a neighbour element
/// @param [in]  face_element_connectivity   For each face of element in own_celements, it stores the element it is connected to. The length of this vector is the total number of elements from
/// own_celements, multiplied with the number of faces for each element. To get the connectivity of face 3 of element i of own_celements, use:
/// face_element_connectivity[nb_faces*i + 3]
/// @param [out] face_face_connectivity Stores to local face index for each global face index
/// For other parameters: @see create_face_element_connectivity
void Mesh_API create_face_face_connectivity( const Entities& own_celements, const NodeConnectivity::EntitiesT& entities, const FaceConnectivity::BoolsT& face_has_neighbour, const NodeConnectivity::NodeElementsT& face_element_connectivity, NodeConnectivity::IndicesT& face_face_connectivity);

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ConnectivityData_hpp
