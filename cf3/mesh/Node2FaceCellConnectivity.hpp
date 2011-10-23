// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Node2FaceCellConnectivity_hpp
#define cf3_mesh_Node2FaceCellConnectivity_hpp

#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/UnifiedData.hpp"
#include "common/DynTable.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  class Link;
}
namespace mesh {

  class Region;
  class Geometry;

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between nodes and their adjacent elements
/// and provides a convenient API to access the data
class Mesh_API Node2FaceCellConnectivity : public common::Component
{
public:

  typedef boost::shared_ptr<Node2FaceCellConnectivity> Ptr;
  typedef boost::shared_ptr<Node2FaceCellConnectivity const> ConstPtr;

  /// Contructor
  /// @param name of the component
  Node2FaceCellConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~Node2FaceCellConnectivity() {}

  /// Get the class name
  static std::string type_name () { return "Node2FaceCellConnectivity"; }

  /// setup the node to element connectivity
  /// This function calls
  /// - set_elements(elements_range)
  /// - build_connectivity
  /// They could be called seperately if wanted
  /// @post all access functions can be used after setup
  /// @param [in] region in which the elements are connected to the nodes.
  void setup(Region& region);

  /// @return lookup table for node to face_cell_connectivity
  UnifiedData& face_cell_connectivity() {  return *m_face_cell_connectivity; }
  const UnifiedData& face_cell_connectivity() const {  return *m_face_cell_connectivity; }

  /// Build the connectivity table
  /// Build the connectivity table as a DynTable<Uint>
  /// @pre set_nodes() and set_elements() must have been called
  void build_connectivity();

  /// const access to the node to element connectivity table in unified indices
  common::DynTable<Uint>& connectivity() { return *m_connectivity; }
  const common::DynTable<Uint>& connectivity() const { return *m_connectivity; }

  Uint size() const { return connectivity().size(); }
//private: //functions

  /// set the nodes for the node to element connectivity
  /// @param [in] nodes the nodes component to find connected elements of
  void set_nodes(Geometry& nodes);

private: // data

  /// link to the nodes component
  boost::shared_ptr<common::Link> m_nodes;

  /// unified view of the elements
  UnifiedData::Ptr m_face_cell_connectivity;

  /// Actual connectivity table
  common::DynTable<Uint>::Ptr m_connectivity;

}; // Node2FaceCellConnectivity

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ConnectivityData_hpp
