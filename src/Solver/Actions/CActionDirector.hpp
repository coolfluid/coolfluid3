// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CActionDirector_hpp
#define CF_Solver_Actions_CActionDirector_hpp

#include "Common/CAction.hpp"

#include "LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

/// Executes a series of actions, configured through a list of URIs to the actions to execute
/// Actions are passed through the "ActionList" option and will be executed in the order they are listed
class Solver_Actions_API CActionDirector : public Common::CAction
{
public: // typedefs

  typedef boost::shared_ptr<CActionDirector> Ptr;
  typedef boost::shared_ptr<CActionDirector const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CActionDirector ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "CActionDirector"; }
  
  /// CAction implementation
  virtual void execute();

private:

  /// Called when the list of actions is configured
  /// This caches pointers to the actions, so the URIs must be valid
  void trigger_actions();
  
  /// list with pointers to the actions to execute
  std::vector< boost::weak_ptr<Common::CAction> > m_actions;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_CActionDirector_hpp
