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

namespace cf3 {
  namespace common
  {
    class Link;
  }
namespace mesh {

  class Connectivity;

////////////////////////////////////////////////////////////////////////////////

/// Elements component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API Elements : public Entities {

public: // typedefs

public: // functions

  /// Contructor
  /// @param name of the component
  Elements ( const std::string& name );

  /// Virtual destructor
  virtual ~Elements();

  /// Get the class name
  static std::string type_name () { return "Elements"; }

  /// Access to the connectivity table
  Connectivity& node_connectivity() const;

  /// return the number of elements
  virtual Uint size() const;

  virtual common::TableConstRow<Uint>::type get_nodes(const Uint elem_idx) const;

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Elements_hpp
