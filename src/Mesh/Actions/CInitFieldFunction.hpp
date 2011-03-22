// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CInitFieldFunction_hpp
#define CF_Mesh_CInitFieldFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/VectorialFunction.hpp"

#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh { 
  class CField;
namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CInitFieldFunction : public CMeshTransformer
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
  
  Math::VectorialFunction  m_function;
  
  boost::weak_ptr<CField> m_field;
  
}; // end CInitFieldFunction


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CInitFieldFunction_hpp
