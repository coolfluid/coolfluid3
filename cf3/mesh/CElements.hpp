// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CElements_hpp
#define cf3_mesh_CElements_hpp

////////////////////////////////////////////////////////////////////////////////


#include "mesh/CEntities.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/CConnectivity.hpp"

namespace cf3 {
  namespace common
  {
    class CLink;
  }
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// CElements component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CElements : public CEntities {

public: // typedefs

  typedef boost::shared_ptr<CElements> Ptr;
  typedef boost::shared_ptr<CElements const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CElements ( const std::string& name );

  /// Initialize uding the given type
  virtual void initialize(const std::string& element_type_name);

  /// Initialize the CElements using the given type and set the nodes
  virtual void initialize(const std::string& element_type_name, Geometry& geo);

  /// Set nodes
  virtual void assign_geometry(Geometry& geo);

  /// Virtual destructor
  virtual ~CElements();

  /// Get the class name
  static std::string type_name () { return "CElements"; }

  /// Access to the connectivity table
  CConnectivity& node_connectivity() const;

  /// return the number of elements
  virtual Uint size() const { return node_connectivity().size(); }

  virtual CTable<Uint>::ConstRow get_nodes(const Uint elem_idx) const;

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CElements_hpp
