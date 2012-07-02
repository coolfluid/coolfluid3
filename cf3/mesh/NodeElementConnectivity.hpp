// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_NodeElementConnectivity_hpp
#define cf3_mesh_NodeElementConnectivity_hpp

#include "mesh/Elements.hpp"
#include "mesh/UnifiedData.hpp"
#include "common/DynTable.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
  class Link;
}
namespace mesh {

  class Region;
  class Dictionary;
  

////////////////////////////////////////////////////////////////////////////////

/// Stores connectivity data between nodes and their adjacent elements
/// and provides a convenient API to access the data
class Mesh_API NodeElementConnectivity : public common::Component
{
public:

  /// Contructor
  /// @param name of the component
  NodeElementConnectivity ( const std::string& name );

  /// Virtual destructor
  virtual ~NodeElementConnectivity() {}

  /// Get the class name
  static std::string type_name () { return "NodeElementConnectivity"; }

  /// setup the node to element connectivity
  /// This function calls
  /// - set_elements(elements_range)
  /// - build_connectivity
  /// They could be called seperately if wanted
  /// @post all access functions can be used after setup
  /// @param [in] region in which the elements are connected to the nodes.
  void setup(Region& region);

  /// Build the connectivity table
  /// Build the connectivity table as a DynTable<Uint>
  /// @pre set_nodes() and set_elements() must have been called
  void build_connectivity();

  UnifiedData& elements() { return *m_elements; }
  const UnifiedData& elements() const { return *m_elements; }


  /// const access to the node to element connectivity table in unified indices
  common::DynTable<Uint>& connectivity() { return *m_connectivity; }
  const common::DynTable<Uint>& connectivity() const { return *m_connectivity; }

private: //functions

  /// set the nodes for the node to element connectivity
  /// @param [in] nodes the nodes component to find connected elements of
  void set_nodes(Dictionary& nodes);

private: // data

  /// link to the nodes component
  Handle<common::Link> m_nodes;

  /// unified view of the elements
  Handle< UnifiedData > m_elements;

  /// Actual connectivity table
  Handle< common::DynTable<Uint> > m_connectivity;

}; // NodeElementConnectivity

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ConnectivityData_hpp
