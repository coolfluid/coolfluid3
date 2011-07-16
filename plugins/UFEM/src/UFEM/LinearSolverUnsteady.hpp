// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_LinearSolverUnsteady_hpp
#define CF_UFEM_LinearSolverUnsteady_hpp

#include "LinearSolver.hpp"
#include "LibUFEM.hpp"

namespace CF {

namespace UFEM {

/// LinearSolverUnsteady for UFEM problems, allowing dynamic configuration and providing access to
/// * Linear system solver
/// * Physical model
/// * Mesh used
/// * Region to loop over
class UFEM_API LinearSolverUnsteady : public LinearSolver
{
public: // typedefs

  typedef boost::shared_ptr<LinearSolverUnsteady> Ptr;
  typedef boost::shared_ptr<LinearSolverUnsteady const> ConstPtr;

public: // functions
  
  /// Contructor
  /// @param name of the component
  LinearSolverUnsteady ( const std::string& name );
  
  virtual ~LinearSolverUnsteady();

  /// Get the class name
  static std::string type_name () { return "LinearSolverUnsteady"; }
  
  /// Reference to the inverse timestep, linked to the model time step
  Real& invdt();
  
private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // UFEM
} // CF


#endif // CF_UFEM_LinearSolverUnsteady_hpp
