// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CStencilComputerRings_hpp
#define cf3_Mesh_CStencilComputerRings_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>
#include "Mesh/CStencilComputer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {

  class CNodeElementConnectivity;

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API CStencilComputerRings : public CStencilComputer
{
public: // typedefs

  typedef boost::shared_ptr<CStencilComputerRings> Ptr;
  typedef boost::shared_ptr<CStencilComputerRings const> ConstPtr;

public: // functions  
  /// constructor
  CStencilComputerRings( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CStencilComputerRings"; }

  virtual void compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil);

private: // functions

  void configure_mesh();
  
  CNodeElementConnectivity& node2cell() { return *m_node2cell.lock(); }

  void compute_neighbors(std::set<Uint>& included, const Uint unified_elem_idx, const Uint level=0);

private: // data
  
  Uint m_nb_rings;

  boost::weak_ptr<CNodeElementConnectivity> m_node2cell;
  
  std::set<Uint> visited_nodes;
}; // end CStencilComputerRings

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Mesh_Neu_CStencilComputerRings_hpp
