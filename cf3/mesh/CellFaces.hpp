// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CellFaces_hpp
#define cf3_mesh_CellFaces_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "mesh/Entities.hpp"

namespace cf3 {
namespace mesh {

class FaceCellConnectivity;

////////////////////////////////////////////////////////////////////////////////

/// CellFaces component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck
class Mesh_API CellFaces : public Entities {

public: // functions

  /// Contructor
  /// @param name of the component
  CellFaces ( const std::string& name );

  /// Virtual destructor
  virtual ~CellFaces();

  /// Get the class name
  static std::string type_name () { return "CellFaces"; }

//  bool is_bdry(const Uint idx) const;

//  FaceCellConnectivity& cell_connectivity() { return *m_cell_connectivity; }
//  const FaceCellConnectivity& cell_connectivity() const { return *m_cell_connectivity; }

//protected:

//  Handle<FaceCellConnectivity> m_cell_connectivity;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CellFaces_hpp
