// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CBuildArea_hpp
#define CF_Mesh_CBuildArea_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/Actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that creates a P0 space for every element in the mesh
/// @author Willem Deconinck
class Mesh_Actions_API CreateSpaceP0 : public CMeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CreateSpaceP0> Ptr;
    typedef boost::shared_ptr<CreateSpaceP0 const> ConstPtr;

public: // functions
  
  /// constructor
  CreateSpaceP0( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CreateSpaceP0"; }

  virtual void execute();

}; // end CreateSpaceP0


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CBuildArea_hpp
