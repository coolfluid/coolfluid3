// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CStencilComputer_hpp
#define CF_Mesh_CStencilComputer_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>

#include "Common/Component.hpp"
#include "Mesh/CUnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  class CMesh;
  class CElements;

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API CStencilComputer : public Common::Component
{
public: // typedefs

  typedef boost::shared_ptr<CStencilComputer> Ptr;
  typedef boost::shared_ptr<CStencilComputer const> ConstPtr;

public: // functions  
  /// constructor
  CStencilComputer( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CStencilComputer"; }
  
  const CUnifiedData<CElements const>& unified_elements() const { return *m_elements; }

  virtual void compute_stencil(const CElements& elements, const Uint elem_idx, std::vector<Uint>& stencil) = 0;

private: // functions

  void configure_mesh();

protected: // data
  
  boost::weak_ptr<CMesh> m_mesh;
    
  CUnifiedData<CElements const>::Ptr m_elements;
  
  Uint m_min_stencil_size;

}; // end CStencilComputer

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CStencilComputer_hpp
