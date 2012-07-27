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

namespace mesh{ class Field; class Space;}

namespace sdm {
  class DomainDiscretization;
  class ComputeCellJacobianPerturb;

namespace implicit{

////////////////////////////////////////////////////////////////////////////////

/// @brief BackwardEuler implicit system
///
/// ( - dR/dQ + I/dt )   dQ   =   R - dQ/dt
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

private: // fuctions

  /// @brief create field to backup solution, needed for this system
  void create_solution_backup();


private:

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
