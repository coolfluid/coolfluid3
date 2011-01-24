// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CEigenLSS_hpp
#define CF_Solver_CEigenLSS_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Math/MatrixTypes.hpp"

#include "LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// CEigenLSS component class
/// This class stores a linear system for use by proto expressions
/// @author Bart Janssens
class Solver_API CEigenLSS : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CEigenLSS> Ptr;
  typedef boost::shared_ptr<CEigenLSS const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CEigenLSS ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "CEigenLSS"; }    
  
  /// Set the number of equations
  void resize ( Uint nb_dofs );
  
  Uint size() const;
  
  /// Reference to the system matrix
  RealMatrix& matrix();
  
  /// Reference to the RHS vector
  RealVector& rhs();
  
  /// Solve the system and store the result in the field that is set in the SolutionField option
  void solve();
  
private:
  /// System matrix
  RealMatrix m_system_matrix;
  
  /// Right hand side
  RealVector m_rhs;
  
  /// Called when the solution field is changed
  void on_solution_field_change();

};
  
////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF


////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CEigenLSS_hpp
