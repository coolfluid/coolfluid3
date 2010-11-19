// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CDomain_hpp
#define CF_Solver_CDomain_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// CDomain component class
/// CDomain stores the meshes
/// @author Tiago Quintino
class Solver_API CDomain : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CDomain> Ptr;
  typedef boost::shared_ptr<CDomain const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CDomain ( const CName& name );

  /// Virtual destructor
  virtual ~CDomain();

  /// Get the class name
  static std::string type_name () { return "CDomain"; }

  /// Configuration Options
  static void define_config_properties ( Common::PropertyList& options ) {}

  // functions specific to the CDomain component
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CDomain_hpp
