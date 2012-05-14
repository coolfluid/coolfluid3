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

//////////////////////////////////////////////////////////////////////////////

/// @brief Compute the stencil around an element, consisting of rings of neighboring cells
/// @author Willem Deconinck
class Mesh_API StencilComputerRings : public StencilComputer {

public: // functions  
  /// constructor
  StencilComputerRings( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "StencilComputerRings"; }

  virtual void compute_stencil(const SpaceElem& element, std::vector<SpaceElem>& stencil);

private: // functions

  void compute_neighbors(std::set<SpaceElem>& included, const SpaceElem& element, const Uint level=0);

private: // data
  
  Uint m_nb_rings;
  
  std::set<Uint> visited_nodes;
}; // end StencilComputerRings

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_StencilComputerRings_hpp
