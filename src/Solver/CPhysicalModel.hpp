// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CPhysicalModel_hpp
#define CF_Solver_CPhysicalModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Component providing information about the physics, i.e. number of equations, variables, ...
/// @author Bart Janssens
class Solver_API CPhysicalModel : public Common::Component {

public: //typedefs

  typedef boost::shared_ptr<CPhysicalModel> Ptr;
  typedef boost::shared_ptr<CPhysicalModel const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CPhysicalModel ( const CName& name );

  /// Virtual destructor
  virtual ~CPhysicalModel();

  /// Get the class name
  static std::string type_name () { return "CPhysicalModel"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options );
  
  //////////////////////////////////
  // CPhysicalModel specific
  /////////////////////////////////
  
  /// Dimensionality of the problem, i.e. the number of components for the spatial coordinates
  Uint dimensions() const;
  
  /// Degrees of freedom
  Uint nb_dof() const;

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}
}; // CPhysicalModel

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CPhysicalModel_hpp
