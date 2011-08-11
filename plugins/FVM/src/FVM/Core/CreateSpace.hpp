// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_CreateSpace_hpp
#define CF_FVM_Core_CreateSpace_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshTransformer.hpp"
#include "FVM/Core/LibCore.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace FVM {
namespace Core {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines an action that creates a space in the mesh
/// with FVM shape functions, and a configurable order of polynomial.
/// Default polynomial order = 0.
/// that returns information about the mesh
/// @author Willem Deconinck
class FVM_Core_API CreateSpace : public Mesh::CMeshTransformer
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

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_CreateSpace_hpp
