// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_FaceNormals_hpp
#define cf3_solver_actions_FaceNormals_hpp

#include "LibActions.hpp"

#include "Proto/ProtoAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

/// Build the normals on faces, using a Proto expression
class solver_actions_API FaceNormals : public Proto::ProtoAction
{
public: // functions
  /// Contructor
  /// @param name of the component
  FaceNormals ( const std::string& name );

  /// Virtual destructor
  virtual ~FaceNormals() {}

  /// Get the class name
  static std::string type_name () { return "FaceNormals"; }
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_FaceNormals_hpp
