// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CCellFaces_hpp
#define CF_Mesh_CCellFaces_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CCellFaces component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck
class Mesh_API CCellFaces : public CEntities {

public: // typedefs

  typedef boost::shared_ptr<CCellFaces> Ptr;
  typedef boost::shared_ptr<CCellFaces const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CCellFaces ( const std::string& name );

  /// Virtual destructor
  virtual ~CCellFaces();

  /// Get the class name
  static std::string type_name () { return "CCellFaces"; }

  /// return the number of elements
  virtual Uint size() const { return m_cell_connectivity->size(); }

  virtual CTable<Uint>::ConstRow get_nodes(const Uint face_idx) const;

  bool is_bdry(const Uint idx) { return m_cell_connectivity->is_bdry_face()[idx]; }

  CFaceCellConnectivity& cell_connectivity() { return *m_cell_connectivity; }
  const CFaceCellConnectivity& cell_connectivity() const { return *m_cell_connectivity; }

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;


protected:

  CFaceCellConnectivity::Ptr m_cell_connectivity;

  mutable CTable<Uint>::ArrayT m_proxy_nodes;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CCellFaces_hpp
