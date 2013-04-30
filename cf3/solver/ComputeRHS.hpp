// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_ComputeRHS_hpp
#define cf3_solver_ComputeRHS_hpp

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
class solver_API ComputeRHS : public common::Action
{
public:

  /// @brief Contructor
  /// @param name of the component
  ComputeRHS ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeRHS() {}

  /// @brief Get the class name
  static std::string type_name () { return "ComputeRHS"; }

  /// @brief Compute the right hand side and wave speed in configured fields
  virtual void execute();

  /// @brief Loop over cell entities
  /// @return false if these cells are to be skipped
  virtual bool loop_cells(const Handle<mesh::Entities const>& cells);

  /// @brief Compute the complete rhs for a given element, as well as the wave-speeds
  virtual void compute_rhs(const Uint elem_idx, std::vector<RealVector>& rhs, std::vector<Real>& wave_speed);

  /// @brief Compute the complete rhs in a field, as well as wave speeds
  virtual void compute_rhs(mesh::Field& rhs, mesh::Field& wave_speed);

private:

  Handle< mesh::Field > m_rhs;  ///! Right hand side field
  Handle< mesh::Field > m_ws;   ///! Wave speed field

  std::vector< Handle<TermComputer> > m_term_computers;
  std::vector< bool > m_loop_cells;

  std::vector< RealVector > m_tmp_term;
  std::vector< Real > m_tmp_ws;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_ComputeRHS_hpp
