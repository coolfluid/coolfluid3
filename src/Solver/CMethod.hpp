// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CMethod_hpp
#define CF_Solver_CMethod_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// Solver now stores:
///   - regions which subdivide in subregions
///   - arrays containing coordinates, variables, ...
/// @author Tiago Quintino
/// @author Willem Deconinck
class Solver_API CMethod : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CMethod> Ptr;
  typedef boost::shared_ptr<CMethod const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMethod ( const std::string& name );

  /// Virtual destructor
  virtual ~CMethod();

  /// Get the class name
  static std::string type_name () { return "CMethod"; }

  // functions specific to the CMethod component
  
  // Signal run_operation
  void run_operation( Common::XmlNode& node ) {}
  
  CMethod& operation(const std::string& name);
  
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CMethod_hpp
