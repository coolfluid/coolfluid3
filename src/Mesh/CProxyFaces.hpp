// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CProxyFaces_hpp
#define CF_Mesh_CProxyFaces_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CEntities.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CProxyFaces component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck
class Mesh_API CProxyFaces : public CEntities {

public: // typedefs

  typedef boost::shared_ptr<CProxyFaces> Ptr;
  typedef boost::shared_ptr<CProxyFaces const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CProxyFaces ( const std::string& name );
    
  /// Virtual destructor
  virtual ~CProxyFaces();

  /// Get the class name
  static std::string type_name () { return "CProxyFaces"; }

  /// return the number of elements
  virtual Uint size() const { return m_cell_connectivity->size(); }
  
  virtual CTable<Uint>::ConstRow get_nodes(const Uint face_idx);
  
  bool is_bdry(const Uint idx) { return m_cell_connectivity->is_bdry_face()[idx]; }
  
  CFaceCellConnectivity& cell_connectivity() { return *m_cell_connectivity; }
  const CFaceCellConnectivity& cell_connectivity() const { return *m_cell_connectivity; }
  
  RealMatrix get_coordinates(const Uint elem_idx);

  void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;
  
  
protected:
  
  CFaceCellConnectivity::Ptr m_cell_connectivity;
  
  CTable<Uint>::ArrayT m_proxy_nodes;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CProxyFaces_hpp
