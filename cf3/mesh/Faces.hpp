// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Faces_hpp
#define cf3_mesh_Faces_hpp

////////////////////////////////////////////////////////////////////////////////


#include "mesh/Elements.hpp"

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// Faces component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API Faces : public Elements {

public: // functions

  /// Contructor
  /// @param name of the component
  Faces ( const std::string& name );

  /// Initialize the Faces using the given type
  virtual void initialize(const std::string& element_type_name, Dictionary& geometry);
    
  /// Virtual destructor
  virtual ~Faces();

  /// Get the class name
  static std::string type_name () { return "Faces"; }

};

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Faces_hpp
