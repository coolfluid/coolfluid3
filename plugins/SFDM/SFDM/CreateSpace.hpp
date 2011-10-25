// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_CreateSpace_hpp
#define CF_SFDM_CreateSpace_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"
#include "SFDM/LibSFDM.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines an action that creates a space in the mesh
/// with SFDM shape functions, and a configurable order of polynomial.
/// Default polynomial order = 0.
/// that returns information about the mesh
/// @author Willem Deconinck
class SFDM_API CreateSpace : public mesh::MeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<CreateSpace> Ptr;
    typedef boost::shared_ptr<CreateSpace const> ConstPtr;

public: // functions
  
  /// constructor
  CreateSpace( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CreateSpace"; }

  virtual void execute();

}; // end CreateSpace


////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_CreateSpace_hpp
