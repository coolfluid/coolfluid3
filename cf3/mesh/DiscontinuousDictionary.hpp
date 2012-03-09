// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_DiscontinuousDictionary_hpp
#define cf3_mesh_DiscontinuousDictionary_hpp

#include "math/MatrixTypes.hpp"
#include "mesh/Dictionary.hpp"

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component that holds Fields of the same type (topology and space)
/// @author Willem Deconinck
class Mesh_API DiscontinuousDictionary : public mesh::Dictionary {

public: // functions

  /// Contructor
  /// @param name of the component
  DiscontinuousDictionary ( const std::string& name );

  /// Virtual destructor
  virtual ~DiscontinuousDictionary();

  virtual void create_connectivity_in_space();
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

#endif // cf3_mesh_DiscontinuousDictionary_hpp
