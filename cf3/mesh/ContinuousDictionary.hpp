// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ContinuousDictionary_hpp
#define cf3_mesh_ContinuousDictionary_hpp

#include "math/MatrixTypes.hpp"
#include "mesh/Dictionary.hpp"

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component that holds Fields of the same type (topology and space)
/// @author Willem Deconinck
class Mesh_API ContinuousDictionary : public mesh::Dictionary {

public: // functions

  /// Get the class name
  static std::string type_name () { return "ContinuousDictionary"; }

  /// Contructor
  /// @param name of the component
  ContinuousDictionary ( const std::string& name );

  /// Virtual destructor
  virtual ~ContinuousDictionary();

  virtual void rebuild_spaces_from_geometry();

  virtual void rebuild_node_to_element_connectivity();

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#endif // cf3_mesh_ContinuousDictionary_hpp
