// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_NodeValence_hpp
#define cf3_solver_actions_NodeValence_hpp

#include "LibActions.hpp"

#include "Proto/ProtoAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

/// Compute the valence (number of adjacent cells) for each node
class solver_actions_API NodeValence : public Proto::ProtoAction
{
public: // functions
  /// Contructor
  /// @param name of the component
  NodeValence ( const std::string& name );

  /// Virtual destructor
  virtual ~NodeValence() {}

  /// Get the class name
  static std::string type_name () { return "NodeValence"; }
  
  virtual void on_regions_set();
  
  virtual void execute();
  
private:
  // Action to zero the field first
  Handle<common::Action> m_zero_field;
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_NodeValence_hpp
