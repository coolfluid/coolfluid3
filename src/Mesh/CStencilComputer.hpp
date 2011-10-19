// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CStencilComputer_hpp
#define cf3_Mesh_CStencilComputer_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "Common/Component.hpp"
#include "Mesh/CUnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {

  class CMesh;
  class CElements;

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API CStencilComputer : public common::Component
{
public: // typedefs

  typedef boost::shared_ptr<CStencilComputer> Ptr;
  typedef boost::shared_ptr<CStencilComputer const> ConstPtr;

public: // functions  
  /// constructor
  CStencilComputer( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CStencilComputer"; }
  
  CUnifiedData& unified_elements() { return *m_elements; }

  virtual void compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil) = 0;

  void set_mesh(CMesh& mesh);

private: // functions

  void configure_mesh();

protected: // data
  
  boost::weak_ptr<CMesh> m_mesh;
    
  CUnifiedData::Ptr m_elements;
  
  Uint m_min_stencil_size;

}; // end CStencilComputer

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Mesh_Neu_CStencilComputer_hpp
