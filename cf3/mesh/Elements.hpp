// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Elements_hpp
#define cf3_mesh_Elements_hpp

////////////////////////////////////////////////////////////////////////////////


#include "mesh/Entities.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Connectivity.hpp"

namespace cf3 {
  namespace common
  {
    class Link;
  }
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// Elements component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API Elements : public Entities {

public: // typedefs

  typedef boost::shared_ptr<Elements> Ptr;
  typedef boost::shared_ptr<Elements const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Elements ( const std::string& name );

  /// Initialize uding the given type
  virtual void initialize(const std::string& element_type_name);

  /// Initialize the Elements using the given type and set the nodes
  virtual void initialize(const std::string& element_type_name, Geometry& geo);

  /// Set nodes
  virtual void assign_geometry(Geometry& geo);

  /// Virtual destructor
  virtual ~Elements();

  /// Get the class name
  static std::string type_name () { return "Elements"; }

  /// Access to the connectivity table
  Connectivity& node_connectivity() const;

  /// return the number of elements
  virtual Uint size() const { return node_connectivity().size(); }

  virtual Table<Uint>::ConstRow get_nodes(const Uint elem_idx) const;

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Elements_hpp
