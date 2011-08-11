// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_BuildGhostStates_hpp
#define CF_FVM_Core_BuildGhostStates_hpp

#include "FVM/Core/LibCore.hpp"

#include "Mesh/CMeshTransformer.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace FVM {
namespace Core {

class FVM_Core_API BuildGhostStates : public Mesh::CMeshTransformer
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BuildGhostStates> Ptr;
  typedef boost::shared_ptr<BuildGhostStates const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  BuildGhostStates ( const std::string& name );

  /// Virtual destructor
  virtual ~BuildGhostStates() {}

  /// Get the class name
  static std::string type_name () { return "BuildGhostStates"; }
  
  /// execute the action
  virtual void execute ();

private: // helper functions
  
  void recursive_build_ghost_states(Component& parent);
  
private: // data
  
};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_BuildGhostStates_hpp
