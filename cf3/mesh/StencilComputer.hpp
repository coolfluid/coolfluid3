// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_StencilComputer_hpp
#define cf3_mesh_StencilComputer_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "common/Component.hpp"
#include "mesh/UnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Mesh;
  class Elements;

//////////////////////////////////////////////////////////////////////////////

/// This class defines neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API StencilComputer : public common::Component
{
public: // typedefs

  
  

public: // functions  
  /// constructor
  StencilComputer( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "StencilComputer"; }
  
  UnifiedData& unified_elements() { return *m_elements; }

  virtual void compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil) = 0;

  void set_mesh(Mesh& mesh);

private: // functions

  void configure_mesh();

protected: // data
  
  Handle<Mesh> m_mesh;
    
  Handle< UnifiedData > m_elements;
  
  Uint m_min_stencil_size;

}; // end StencilComputer

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_StencilComputer_hpp
