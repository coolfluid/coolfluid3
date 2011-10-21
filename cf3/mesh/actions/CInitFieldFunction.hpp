// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CInitFieldFunction_hpp
#define cf3_mesh_CInitFieldFunction_hpp

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
class mesh_actions_API CInitFieldFunction : public MeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CInitFieldFunction> Ptr;
    typedef boost::shared_ptr<CInitFieldFunction const> ConstPtr;

public: // functions
  
  /// constructor
  CInitFieldFunction( const std::string& name );
  
  /// destructor
  virtual ~CInitFieldFunction();
  
  /// Gets the Class name
  static std::string type_name() { return "CInitFieldFunction"; }

  virtual void execute();

private: // functions

  void config_function();

private: // data
  
  math::VectorialFunction  m_function;
  
  boost::weak_ptr<Field> m_field;
  
}; // end CInitFieldFunction


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CInitFieldFunction_hpp
