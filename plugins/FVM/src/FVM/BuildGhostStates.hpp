// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_BuildGhostStates_hpp
#define CF_FVM_BuildGhostStates_hpp

#include "FVM/LibFVM.hpp"

#include "Mesh/CMeshTransformer.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace FVM {

class FVM_API BuildGhostStates : public Mesh::CMeshTransformer
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
  virtual ~BuildGhostStates() {};

  /// Get the class name
  static std::string type_name () { return "BuildGhostStates"; }
  
  /// execute the action
  virtual void execute ();

private: // helper functions
  
  void recursive_build_ghost_states(Component& parent);
  
private: // data
  
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_BuildGhostStates_hpp
