// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_PDE_hpp
#define cf3_solver_PDE_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

// Forward declarations
namespace cf3 {
  namespace common {
    class ActionDirector;
  }
  namespace mesh {
    class Dictionary;
    class Field;
  }
  namespace solver {
    class ComputeRHS;
    class TermComputer;
    class BC;
    class Time;
  }
  namespace solver {
    class BoundaryConditions;
    class Solver;
    class TimeIntegrationStepComputer;
  }
}

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// @brief PDE component
/// @author Willem Deconinck
/// This component is a base component for implementing
/// different PDE.
/// A PDE component works with one fieldsionary, and
/// knows how to compute the right-hand-side of the equations
/// it represents.
/// PDE is made of terms, term-computers, and configuration options
class solver_API PDE : public common::Component {

public: // functions

  /// @brief Contructor
  /// @param name of the component
  PDE ( const std::string& name );

  /// @brief Virtual destructor
  virtual ~PDE();

  /// @brief Get the class name
  static std::string type_name () { return "PDE"; }

  /// @brief Number of equations describing this PDE
  Uint nb_eqs() const { return m_nb_eqs; }

  /// @brief Number of dimensions describing this PDE
  Uint nb_dim() const { return m_nb_dim; }

  virtual std::string solution_variables() const;

  /// @brief Handle to the fields
  const Handle<mesh::Dictionary>& fields() { return m_fields; }

  /// @brief Handle to the configured solution
  const Handle<mesh::Field>& solution() { return m_solution; }

  /// @brief Handle to the configured rhs
  const Handle<mesh::Field>& rhs() { return m_rhs; }

  /// @brief Handle to the configured wave_speed
  const Handle<mesh::Field>& wave_speed() { if (is_null(m_wave_speed)) throw common::BadValue(FromHere(), ""); return m_wave_speed; }

  /// @brief Handle to the boundary fields
  const Handle<mesh::Dictionary>& bdry_fields() { return m_bdry_fields; }

  /// @brief Handle to the configured solution
  const Handle<mesh::Field>& bdry_solution() { return m_bdry_solution; }

  /// @brief Handle to the configured solution
  const Handle<mesh::Field>& bdry_solution_gradient() { return m_bdry_solution_gradient; }

  /// @brief Handle to the time component
  const Handle<solver::Time> time() { return m_time; }

  /// @brief Handle to the ODE right-hand-side computer
  ///
  /// dQ/dt = R( Q )
  const Handle<solver::ComputeRHS>& rhs_computer() { return m_rhs_computer; }

  /// @brief Action that executes all contained boundary conditions
  const Handle<common::ActionDirector>& bc() { return m_bc; }

  /// @brief Create a time component, making this unsteady in time
  Handle<solver::Time> add_time();

  /// @brief Create a term, configure it, and create a term-computer
  Handle<solver::TermComputer> add_term( const std::string& term_name,
                                         const std::string& term_computer );

  /// @brief Create a boundary condition, configure it, and create a bc-computer
  Handle<solver::BC> add_bc( const std::string& bc_name,
                             const std::string& bc_type,
                             const std::vector< Handle<Component> >& regions );

  virtual void configure(const Handle<common::Component>& component);

  /// @brief create necessary fields when fields is configured
  virtual void create_fields();

  /// @brief create necessary bdry_fields when bdry_fields is configured
  virtual void create_bdry_fields();

public: // signals

  void signal_add_term( common::SignalArgs& args );
  void signature_add_term( common::SignalArgs& args );

  void signal_add_bc( common::SignalArgs& args );
  void signature_add_bc( common::SignalArgs& args );

protected: // data

  Uint m_nb_dim;
  Uint m_nb_eqs;
  Handle<solver::ComputeRHS>                    m_rhs_computer;
  Handle<common::ActionDirector>                m_bc;
  Handle<solver::Time>                          m_time;

  Handle<mesh::Dictionary>                      m_fields;
  Handle<mesh::Field>                           m_solution;
  Handle<mesh::Field>                           m_rhs;
  Handle<mesh::Field>                           m_wave_speed;

  Handle<mesh::Dictionary>                      m_bdry_fields;
  Handle<mesh::Field>                           m_bdry_solution;
  Handle<mesh::Field>                           m_bdry_solution_gradient;

};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_PDE_hpp
