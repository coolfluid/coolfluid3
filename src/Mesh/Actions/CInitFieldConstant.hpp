// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CInitFieldConstant_hpp
#define CF_Mesh_CInitFieldConstant_hpp

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

  boost::weak_ptr<CField> m_field;
  
}; // end CInitFieldConstant


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CInitFieldConstant_hpp
