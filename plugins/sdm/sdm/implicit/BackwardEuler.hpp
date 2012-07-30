// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/implicit/BackwardEuler.hpp
/// @author Willem Deconinck, Matteo Parsani
///
/// This file includes the BackwardEuler component class.

#ifndef cf3_sdm_implicit_BackwardEuler_hpp
#define cf3_sdm_implicit_BackwardEuler_hpp

#include "math/MatrixTypes.hpp"
#include "sdm/implicit/LibImplicit.hpp"

#include "sdm/System.hpp"

namespace cf3 {

namespace solver{ class Solver; }
namespace mesh{ class Field; class Space;}

namespace sdm {
  class DomainDiscretization;
  class ComputeCellJacobianPerturb;

namespace implicit{

///////////////////////////////////////////////////////////////////////////////

// Backward Euler, to be solved by iterative non-linear system solver
//             n
// /     d R (Q )         I    \    n + 1,k + 1    n + 1,k
// |  -  --------  +   ------- |  (Q            - Q       )
// \       d Q         Delta t /
//                                            n + 1,k    n
//                            n + 1,k       Q        - Q
//                      =  R(Q       )   -  -------------
//                                             Delta t
//
// n = time-level
// k = iterative sweep

/// @brief BackwardEuler implicit system
///
/// @f[ \left( - \frac{\partial R}{\partial Q}(Q^n) + \frac{I}{\Delta t} \right)
/// \ (Q^{n+1,k+1}-Q^{n+1,k}) = R(Q^{n+1,k}) - \frac{Q^{n+1,k}-Q^{n}}{\Delta t} @f]
/// with n the time-levels, and k the iterative sweeps
///
/// This component provides functions to compute the LHS and
/// the RHS on a per element basis
///
/// These functions are typically called by a matrix assembler
/// or an LUSGS iterative solver
///
/// @author Willem Deconinck, Matteo Parsani
class sdm_implicit_API BackwardEuler : public sdm::System {

public: // functions

  /// @brief Type name
  static std::string type_name () { return "BackwardEuler"; }

  /// @brief Contructor
  /// @param name of the component
  BackwardEuler ( const std::string& name );

  /// @brief Destructor
  virtual ~BackwardEuler() {}

  // Prepare the system before looping
  virtual void prepare();

  // loop cells
  virtual bool loop_cells(const Handle<const cf3::mesh::Cells> &cells);

  // compute the left-hand-side
  virtual void compute_lhs(const Uint elem, RealMatrix& lhs);

  // compute the right-hand-side
  virtual void compute_rhs(const Uint elem, RealVector& rhs);

  // update solution with the solved unknowns
  virtual Real update(const Uint elem, const RealVector& unknowns);

  // Number of columns of the system left-hand-side matrix
  virtual Uint nb_cols() const;

  // Number of rows of the system matrix
  virtual Uint nb_rows() const;

private: // fuctions

  /// @brief create field to backup solution, needed for this system
  void create_solution_backup();

  void configure();

private:

  Handle<solver::Solver> m_solver;

  Handle<DomainDiscretization> m_domain_discretization;

  Handle<ComputeCellJacobianPerturb> m_compute_jacobian;

  Handle<mesh::Field> m_solution_backup;

  Handle<mesh::Field> m_solution;

  Handle<mesh::Field> m_residual;

  Handle<mesh::Field> m_update_coeff;

  Handle<mesh::Space const> m_space;

  Uint m_nb_sol_pts;
  Uint m_nb_vars;
};

////////////////////////////////////////////////////////////////////////////////

} // implicit
} // sdm
} // cf3

#endif // cf3_sdm_implicit_BackwardEuler_hpp
