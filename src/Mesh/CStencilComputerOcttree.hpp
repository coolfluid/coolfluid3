// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CStencilComputerOcttree_hpp
#define cf3_Mesh_CStencilComputerOcttree_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CStencilComputer.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {

  class CMesh;
  class COcttree;

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API CStencilComputerOcttree : public CStencilComputer
{
public: // typedefs

  typedef boost::shared_ptr<CStencilComputerOcttree> Ptr;
  typedef boost::shared_ptr<CStencilComputerOcttree const> ConstPtr;

public: // functions  
  /// constructor
  CStencilComputerOcttree( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CStencilComputerOcttree"; }

  virtual void compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil);

private: // functions

  void configure_mesh();
  
private: // data
  
  boost::shared_ptr<COcttree> m_octtree;
  
  Uint m_dim;
  Uint m_nb_elems_in_mesh;

}; // end CStencilComputerOcttree

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Mesh_Neu_CStencilComputerOcttree_hpp
