// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CWizard_hpp
#define CF_Solver_CWizard_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Wizard component class
/// Derived classes typically help users configure
/// and setup a simulation
/// @author Willem Deconinck
class Solver_API CWizard : public Common::Component {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CWizard> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CWizard const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CWizard ( const std::string& name );

  /// Virtual destructor
  virtual ~CWizard();

  /// Get the class name
  static std::string type_name () { return "CWizard"; }

  virtual std::string wizard_name() { return "wizard"; }
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CWizard_hpp
