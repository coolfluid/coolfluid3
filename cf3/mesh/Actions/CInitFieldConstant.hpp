// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CInitFieldConstant_hpp
#define cf3_mesh_CInitFieldConstant_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/VectorialFunction.hpp"

#include "mesh/CMeshTransformer.hpp"

#include "mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { 
  class Field;
namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CInitFieldConstant : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CInitFieldConstant> Ptr;
    typedef boost::shared_ptr<CInitFieldConstant const> ConstPtr;

public: // functions
  
  /// constructor
  CInitFieldConstant( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CInitFieldConstant"; }

  virtual void execute();
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;

private: // data

  Real m_constant;

  boost::weak_ptr<Field> m_field;
  
}; // end CInitFieldConstant


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CInitFieldConstant_hpp
