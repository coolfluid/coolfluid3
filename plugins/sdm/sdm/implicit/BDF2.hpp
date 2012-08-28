// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/implicit/BDF2.hpp
/// @author Willem Deconinck, Matteo Parsani
///
/// This file includes the BDF2 component class.

#ifndef cf3_sdm_implicit_BDF2_hpp
#define cf3_sdm_implicit_BDF2_hpp

#include "math/MatrixTypes.hpp"
#include "sdm/implicit/LibImplicit.hpp"

#include "sdm/System.hpp"

namespace cf3 {

namespace common{ class Action; }
namespace solver{ class Solver; }
namespace mesh{ class Field; class Space;}

namespace sdm {
  class DomainDiscretization;
  class ComputeCellJacobianPerturb;
  class TimeIntegrationStepComputer;

namespace implicit{

///////////////////////////////////////////////////////////////////////////////

// Second order backward difference,
// to be solved by iterative non-linear system solver
//                n
// /        d R (Q )         I    \    n + 1,k + 1    n + 1,k
// |  - c1  --------  +   ------- |  (Q            - Q       )
// \          d Q         Delta t /
//                                                              n + 1,k    n
//                          n + 1,k            n    n - 1      Q        - Q
//                =  c1  R(Q       )   + c2  (Q  - Q     )  -  -------------
//                                                                Delta t
//
// n = time-level
// k = iterative sweep
// c1 = ( 1 + tau ) / ( 1 + 2 tau )
// c2 = ( tau*tau ) / ( Delta t^n ( 1 + 2 tau ) )
// tau = ( Delta t^n ) / ( Delta t^{n-1} )

/// @brief Second-order backward difference implicit system
///
/// @ref M. Parsani, G. Ghorbaniasl, C. Lacor, E. Turkel, An implicit high-order spectral difference
///      approach for large eddy simulation, Journal of Computational Physics, Volume 229, Issue 14, 20 July 2010,
///      Pages 5373-5393, ISSN 0021-9991, 10.1016/j.jcp.2010.03.038.
///      (http://www.sciencedirect.com/science/article/pii/S0021999110001579)
///
/// @f[ \left( - c_1\  \frac{\partial R}{\partial Q}(Q^n) + \frac{I}{\Delta t} \right)
/// \ (Q^{n+1,k+1}-Q^{n+1,k}) = c_1 \ R(Q^{n+1,k}) + c_2\ (Q^n - Q^{n-1}) - \frac{Q^{n+1,k}-Q^{n}}{\Delta t} @f]
/// with n the time-levels, and k the iterative sweeps,
/// @f[ \tau = \frac{\Delta t^n}{\Delta t^{n-1}} @f]
/// @f[ c_1 = \frac{1 + \tau}{1 + 2 \tau} @f]
/// @f[ c_2 = \frac{\tau^2}{ \Delta t^n\ (1 + 2 \tau)} @f]
///
/// This component provides functions to compute the LHS and
/// the RHS on a per element basis
///
/// These functions are typically called by a matrix assembler
/// or an LUSGS iterative solver
///
/// @author Willem Deconinck, Matteo Parsani
class sdm_implicit_API BDF2 : public sdm::System {

public: // functions

  /// @brief Type name
  static std::string type_name () { return "BDF2"; }

  /// @brief Contructor
  /// @param name of the component
  BDF2 ( const std::string& name );

  /// @brief Destructor
  virtual ~BDF2() {}

  // Prepare the system before looping
  virtual void prepare();

  // loop cells
  virtual bool loop_cells(const Handle<const cf3::mesh::Entities> &cells);

  // compute the left-hand-side
  virtual void compute_lhs(const Uint elem, RealMatrix& lhs);

  // compute the right-hand-side
  virtual void compute_rhs(const Uint elem, RealVector& rhs);

  // update solution with the solved unknowns
  virtual Real update(const Uint elem, const RealVector& unknowns);

  // perform parallel synchronization
  virtual void synchronize();

  // Number of columns of the system left-hand-side matrix
  virtual Uint nb_cols() const;

  // Number of rows of the system matrix
  virtual Uint nb_rows() const;

private: // fuctions

  /// @brief create field to backup solution, needed for this system
  void create_solution_backups();

  /// @brief create field to backup update_coefficients, needed for this system
  void create_update_coeff_backups();

  /// @brief Auto-configuration based on "m_solver"
  void configure();

  /// @brief coefficient c1
  /// @param [in] dt_n    Current time step
  /// @param [in] dt_nm1  Previous time step
  /// @return coefficient c1
  Real coeff_c1(const Real& dt_n, const Real& dt_nm1) const;

  /// @brief coefficient c2
  /// @param [in] dt_n    Current time step
  /// @param [in] dt_nm1  Previous time step
  /// @return coefficient c2
  Real coeff_c2(const Real& dt_n, const Real& dt_nm1) const;

private:

  /// @brief Delegation to compute a jacobian for one cell
  Handle<ComputeCellJacobianPerturb> m_compute_jacobian;

  /// @brief Component that computes the space-residual for one cell
  Handle<DomainDiscretization> m_domain_discretization;

  /// @brief Storage of the solution at time level "n-1"
  Handle<mesh::Field> m_solution_previous;

  /// @brief Storage of the solution at time level "n"
  Handle<mesh::Field> m_solution_backup;

  /// @brief Storage of solution to be updated to time level "n+1"
  Handle<mesh::Field> m_solution;

  /// @brief Storage of the space-residual
  Handle<mesh::Field> m_residual;

  /// @brief Storage of wave-speeds
  Handle<mesh::Field> m_wave_speed;

  /// @brief Storage for local time-steps
  Handle<mesh::Field> m_update_coeff;

  /// @brief Storage for local time-steps at time level "n-1"
  Handle<mesh::Field> m_update_coeff_previous;

  /// @brief Temporary component to hold the "space" of a cell patch
  Handle<mesh::Space const> m_space;

  Handle<TimeIntegrationStepComputer> m_time_step_computer;

  /// @brief Temporary storage for number of solution points per cell
  Uint m_nb_sol_pts;

  /// @brief Temporary storage for number of solution variables
  Uint m_nb_vars;
};

////////////////////////////////////////////////////////////////////////////////

} // implicit
} // sdm
} // cf3

#endif // cf3_sdm_implicit_BDF2_hpp
