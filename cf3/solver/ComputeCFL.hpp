// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_ComputeCFL_hpp
#define cf3_solver_ComputeCFL_hpp

#include "common/Action.hpp"
#include "math/MatrixTypes.hpp"
#include "solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

// Forward declares
namespace cf3 {
  namespace mesh {
    class Entities;
    class Field;
    class Dictionary;
  }
  namespace solver {
    class TermComputer;
  }
}

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// @brief Compute Right-Hand-Side of a PDE
/// @author Willem Deconinck
class solver_API ComputeCFL : public common::Action
{
public:

  /// @brief Contructor
  /// @param name of the component
  ComputeCFL ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeCFL() {}

  /// @brief Get the class name
  static std::string type_name () { return "ComputeCFL"; }

  /// @brief Compute the right hand side and wave speed in configured fields
  virtual void execute();

private:

  Handle< mesh::Field > m_time_step;  ///! Time step field
  Handle< mesh::Field > m_wave_speed; ///! Wave speed field
  Handle< mesh::Field > m_cfl;        ///! CFL field

};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_ComputeCFL_hpp
