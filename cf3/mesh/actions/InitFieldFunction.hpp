// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_InitFieldFunction_hpp
#define cf3_mesh_InitFieldFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/VectorialFunction.hpp"

#include "mesh/MeshTransformer.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { 
  class Field;
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class mesh_actions_API InitFieldFunction : public MeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<InitFieldFunction> Ptr;
    typedef boost::shared_ptr<InitFieldFunction const> ConstPtr;

public: // functions
  
  /// constructor
  InitFieldFunction( const std::string& name );
  
  /// destructor
  virtual ~InitFieldFunction();
  
  /// Gets the Class name
  static std::string type_name() { return "InitFieldFunction"; }

  virtual void execute();

private: // functions

  void config_function();

private: // data
  
  math::VectorialFunction  m_function;
  
  boost::weak_ptr<Field> m_field;
  
}; // end InitFieldFunction


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_InitFieldFunction_hpp
