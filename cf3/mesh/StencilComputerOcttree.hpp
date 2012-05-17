// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_StencilComputerOcttree_hpp
#define cf3_mesh_StencilComputerOcttree_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "mesh/StencilComputer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Mesh;
  class Entity;
  class Octtree;

//////////////////////////////////////////////////////////////////////////////

/// This class defines neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API StencilComputerOcttree : public StencilComputer {

public: // functions  
  /// constructor
  StencilComputerOcttree( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "StencilComputerOcttree"; }

  virtual void compute_stencil(const SpaceElem& element, std::vector<SpaceElem>& stencil);

private: // functions

  void configure_octtree();

private: // data
  
  Handle<Octtree> m_octtree;
  
  Uint m_dim;
  Uint m_nb_elems_in_mesh;

  std::vector<Uint> m_octtree_cell;
  RealVector m_centroid;

  std::vector<Entity> m_stencil;


}; // end StencilComputerOcttree

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_StencilComputerOcttree_hpp
