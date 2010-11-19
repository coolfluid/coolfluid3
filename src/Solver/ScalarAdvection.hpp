// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_ScalarAdvection_hpp
#define CF_Solver_ScalarAdvection_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Wizard to setup a scalar advection simulation
/// @author Tiago Quintino
class Solver_API ScalarAdvection : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<ScalarAdvection> Ptr;
  typedef boost::shared_ptr<ScalarAdvection const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ScalarAdvection ( const CName& name );

  /// Virtual destructor
  virtual ~ScalarAdvection();

  /// Get the class name
  static std::string type_name () { return "ScalarAdvection"; }

  /// Configuration Options
  static void define_config_properties ( Common::PropertyList& options );

  // functions specific to the ScalarAdvection component
  
  /// Signal run_operation
  void run_wizard ( Common::XmlNode& node );
  
  ScalarAdvection& operation(const std::string& name);
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( ScalarAdvection* self );

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_ScalarAdvection_hpp
