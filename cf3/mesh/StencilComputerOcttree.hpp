// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_StencilComputerOcttree_hpp
#define cf3_mesh_StencilComputerOcttree_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/StencilComputer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Mesh;
  class Octtree;

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API StencilComputerOcttree : public StencilComputer
{
public: // typedefs

  typedef boost::shared_ptr<StencilComputerOcttree> Ptr;
  typedef boost::shared_ptr<StencilComputerOcttree const> ConstPtr;

public: // functions  
  /// constructor
  StencilComputerOcttree( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "StencilComputerOcttree"; }

  virtual void compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil);

private: // functions

  void configure_mesh();
  
private: // data
  
  boost::shared_ptr<Octtree> m_octtree;
  
  Uint m_dim;
  Uint m_nb_elems_in_mesh;

}; // end StencilComputerOcttree

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Neu_StencilComputerOcttree_hpp
