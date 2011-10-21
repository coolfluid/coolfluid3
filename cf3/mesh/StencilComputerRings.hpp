// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_StencilComputerRings_hpp
#define cf3_mesh_StencilComputerRings_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>
#include "mesh/StencilComputer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class NodeElementConnectivity;

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API StencilComputerRings : public StencilComputer
{
public: // typedefs

  typedef boost::shared_ptr<StencilComputerRings> Ptr;
  typedef boost::shared_ptr<StencilComputerRings const> ConstPtr;

public: // functions  
  /// constructor
  StencilComputerRings( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "StencilComputerRings"; }

  virtual void compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil);

private: // functions

  void configure_mesh();
  
  NodeElementConnectivity& node2cell() { return *m_node2cell.lock(); }

  void compute_neighbors(std::set<Uint>& included, const Uint unified_elem_idx, const Uint level=0);

private: // data
  
  Uint m_nb_rings;

  boost::weak_ptr<NodeElementConnectivity> m_node2cell;
  
  std::set<Uint> visited_nodes;
}; // end StencilComputerRings

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Neu_StencilComputerRings_hpp
