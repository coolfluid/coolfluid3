// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
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

public: // functions

  /// Contructor
  /// @param name of the component
  Elements ( const std::string& name );

  /// Virtual destructor
  virtual ~Elements();

  /// Get the class name
  static std::string type_name () { return "Elements"; }

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Elements_hpp
