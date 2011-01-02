// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ConnectivityData_hpp
#define CF_Mesh_ConnectivityData_hpp

#include <set>

#include <boost/shared_ptr.hpp>

#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between nodes and their adjacent elements
/// and provides a convenient API to access the data
class Mesh_API CNodeConnectivity : public Common::Component
{
public:

  typedef boost::shared_ptr<CNodeConnectivity> Ptr;
  typedef boost::shared_ptr<CNodeConnectivity const> ConstPtr;
  
  /// Storage for a list of elements
  typedef std::vector<CElements::ConstPtr> ElementsT;

  /// Storage for counted numbers
  typedef std::vector<Uint> CountsT;

  /// Storage for indices into other arrays
  typedef std::vector<Uint> IndicesT;
  
  /// Uniquely refer an element by its CElements and local element index
  typedef std::pair<CElements const*, Uint> ElementReferenceT;
  
  /// Contructor
  /// @param name of the component
  CNodeConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~CNodeConnectivity();

  /// Get the class name
  static std::string type_name () { return "CNodeConnectivity"; }

  // functions specific to the CNodeConnectivity component
  
  /// Initialize the connectivity arrays, based on a range of CElements to consider
  /// Parameters use the convention from the low level interface
  /// @note RangeT is a template argument to allow maximum flexibility with regard to the ranges defined in ComponentPredicates
  /// @see create_celements_vector
  /// @see create_node_element_connectivity
  template<typename RangeT>
  void initialize(const RangeT& celements_range);
  
  /// Allow explicitely supplying the number of nodes
  template<typename RangeT>
  void initialize(const Uint nb_nodes, const RangeT& celements_range);
  
  /// Range of global element indices that use the node with index node_idx.
  boost::iterator_range<IndicesT::const_iterator> node_element_range(const Uint node_idx) const;
  
  /// Convert a global element index to its CElements and local element index
  ElementReferenceT element(const Uint global_element_idx) const;
  
  /// Access to the raw data
  const ElementsT& celements_vector() const { return m_celements_vector; }
  const IndicesT& celements_first_elements() const { return m_celements_first_elements; }
  const IndicesT& node_first_elements() const { return m_node_first_elements; }
  const CountsT& node_element_counts() const { return m_node_element_counts; }
  const IndicesT& node_elements() const { return m_node_elements; }

private: // data
  ElementsT m_celements_vector;
  IndicesT m_celements_first_elements;
  IndicesT m_node_first_elements;
  CountsT m_node_element_counts;
  IndicesT m_node_elements;
}; // CNodeConnectivity

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between element faces and their adjacent elements
/// and provides a convenient API to access the data
class Mesh_API CFaceConnectivity : public Common::Component
{
public:

  typedef boost::shared_ptr<CFaceConnectivity> Ptr;
  typedef boost::shared_ptr<CFaceConnectivity const> ConstPtr;
  
  /// Storage for a list of elements
  typedef CNodeConnectivity::ElementsT ElementsT;

  /// Storage for counted numbers
  typedef CNodeConnectivity::CountsT CountsT;

  /// Storage for indices into other arrays
  typedef CNodeConnectivity::IndicesT IndicesT;

  /// Storage for a list of boolean values
  typedef std::vector<bool> BoolsT;
  
  /// Uniquely refer an element by its CElements and local element index
  typedef CNodeConnectivity::ElementReferenceT ElementReferenceT;

  /// Contructor
  /// @param name of the component
  CFaceConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~CFaceConnectivity();

  /// Get the class name
  static std::string type_name () { return "CFaceConnectivity"; }

  // functions specific to the CFaceConnectivity component
  
  /// Initialize the connectivity arrays so the adjacent face- and element lookups are defined for celements
  /// Parameters use the convention from the low level interface
  /// @see create_celements_vector
  /// @see create_node_element_connectivity
  /// @see create_face_element_connectivity
  /// @see create_face_face_connectivity
  void initialize(const CElements& own_celements, const CNodeConnectivity& node_connectivity);
  
  /// Shortcut assuming the parent is the own_celements
  void initialize(const CNodeConnectivity& node_connectivity);
  
  /// True if the given face of the given element has an adjacent element
  bool has_adjacent_element(const Uint element, const Uint face) const;
  
  /// Get the element that is adjacent to the given face of the given element
  ElementReferenceT adjacent_element(const Uint element, const Uint face) const;
  
  /// Get the face that is adjacent to the given face of the given element
  Uint adjacent_face(const Uint element, const Uint face) const;

private: // data
  const CNodeConnectivity* m_node_connectivity; // normal pointer for performance reasons
  
  /// Number of faces per element
  Uint m_element_nb_faces;
  
  BoolsT m_face_has_neighbour;
  IndicesT m_face_element_connectivity;
  IndicesT m_face_face_connectivity;
}; // CFaceConnectivity

////////////////////////////////////////////////////////////////////////////////

/// Create a vector containing all the CElements in the given range
template <typename RangeT>
void create_celements_vector(const RangeT& range, CFaceConnectivity::ElementsT& celements_vector, CFaceConnectivity::IndicesT& celements_first_elements)
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
    sum += elements.connectivity_table().array().size();
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
void Mesh_API create_node_element_connectivity( const Uint nb_nodes, const CFaceConnectivity::ElementsT& elements, const CFaceConnectivity::IndicesT& celements_first_elements, CFaceConnectivity::IndicesT& node_first_elements, CFaceConnectivity::CountsT& node_element_counts, CFaceConnectivity::IndicesT& node_elements);

/// Calculate the face connectivity data, based on inputs as calculated by create_node_element_connectivity.
/// @param [in] own_celements The CElements for which the connectivity data is needed
/// @param [out] face_has_neighbour For each face of each global element, true if the face has a neighbour element
/// @param [out] face_element_connectivity For each face of element in own_celements, store the element it is connected to. The length of this vector is the total number of elements from
/// own_celements, multiplied with the number of faces for each element. To get the connectivity of face 3 of element i of own_celements, use:
/// face_element_connectivity[nb_faces*i + 3]
void Mesh_API create_face_element_connectivity( const CElements& own_celements, const CFaceConnectivity::ElementsT& celements_vector, const CFaceConnectivity::IndicesT& celements_first_elements, const CFaceConnectivity::IndicesT& node_first_elements, const CFaceConnectivity::CountsT& node_element_counts, const CFaceConnectivity::IndicesT& node_elements, CFaceConnectivity::BoolsT& face_has_neighbour, CFaceConnectivity::IndicesT& face_element_connectivity);

/// Calculate the local face index in adjacent elements
/// @param [out] face_face_connectivity Stores to local face index for each global face index
/// For other parameters: @see create_face_element_connectivity
void Mesh_API create_face_face_connectivity( const CElements& own_celements, const CFaceConnectivity::ElementsT& celements_vector, const CFaceConnectivity::IndicesT& celements_first_elements, const CFaceConnectivity::BoolsT& face_has_neighbour, const CFaceConnectivity::IndicesT& face_element_connectivity, CFaceConnectivity::IndicesT& face_face_connectivity);

template<typename RangeT>
void CNodeConnectivity::initialize (const RangeT& celements_range )
{
  std::set<const CNodes*> nodes_set;
  BOOST_FOREACH(const CElements& elements, celements_range)
    nodes_set.insert(&elements.nodes());
  
  // Total number of nodes in the mesh
  Uint nb_nodes = 0;
  BOOST_FOREACH(const CNodes* nodes, nodes_set)
    nb_nodes += nodes->size();
    
  initialize(nb_nodes, celements_range);
}

template<typename RangeT>
void CNodeConnectivity::initialize(const Uint nb_nodes, const RangeT& celements_range)
{
  create_celements_vector(celements_range, m_celements_vector, m_celements_first_elements);
  create_node_element_connectivity(nb_nodes, m_celements_vector, m_celements_first_elements, m_node_first_elements, m_node_element_counts, m_node_elements);
}

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ConnectivityData_hpp
