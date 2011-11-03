// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

public: // typedefs

  typedef boost::shared_ptr<CellFaces> Ptr;
  typedef boost::shared_ptr<CellFaces const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CellFaces ( const std::string& name );

  /// Virtual destructor
  virtual ~CellFaces();

  /// Get the class name
  static std::string type_name () { return "CellFaces"; }

  /// return the number of elements
  virtual Uint size() const;

  virtual common::TableConstRow<Uint>::type get_nodes(const Uint face_idx) const;

  bool is_bdry(const Uint idx) const;

  FaceCellConnectivity& cell_connectivity() { return *m_cell_connectivity; }
  const FaceCellConnectivity& cell_connectivity() const { return *m_cell_connectivity; }

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;


protected:

  boost::shared_ptr<FaceCellConnectivity> m_cell_connectivity;
  boost::scoped_ptr<common::TableArray<Uint>::type> m_proxy_nodes;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CellFaces_hpp
