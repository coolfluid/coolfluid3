// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Actions_CExtract_hpp
#define cf3_mesh_Actions_CExtract_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"

#include "mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CExtract : public MeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CExtract> Ptr;
    typedef boost::shared_ptr<CExtract const> ConstPtr;

private: // typedefs
  
public: // functions
  
  /// constructor
  CExtract( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CExtract"; }

  virtual void execute();
  
  /// brief description, typically one line
  virtual std::string brief_description() const;
  
  /// extended help that user can query
  virtual std::string help() const;

}; // end CExtract


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CExtract_hpp
