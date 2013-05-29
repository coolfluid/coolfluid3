// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_ComputeFieldGradient_hpp
#define cf3_mesh_actions_ComputeFieldGradient_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Field;

namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Compute the gradient of a field
/// @author Willem Deconinck
///
/// If field contains variables (in order) "a , b , c",
/// then field_gradient will contain variables (in order, e.g. 2D)
/// "da/dx  db/dx  dc/dx  da/dy db/dy dc/dy"
///
/// Shapefunctions are used to compute the gradient.
///
/// @note In the case of a P0 shape-function,
///       the gradient will be thus wrongly calculated to
///       be zero, as variables are piece-wise constant.
///
class mesh_actions_API ComputeFieldGradient : public MeshTransformer
{   
public: // functions
  
  /// constructor
  ComputeFieldGradient( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "ComputeFieldGradient"; }

  virtual void execute();
  
private: // data

  Handle<Field const> m_field;
  Handle<Field>       m_field_gradient;
  std::vector<Real>   m_normal;

}; // end ComputeFieldGradient

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_ComputeFieldGradient_hpp
