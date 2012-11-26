// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_LSSAction_hpp
#define cf3_UFEM_LSSAction_hpp

#include "common/ActionDirector.hpp"
#include "common/OptionURI.hpp"

#include "solver/ActionDirector.hpp"

#include "solver/actions/Proto/BlockAccumulator.hpp"
#include "solver/actions/Proto/DirichletBC.hpp"
#include "solver/actions/Proto/PhysicsConstant.hpp"
#include "solver/actions/Proto/SolutionVector.hpp"
#include "solver/actions/Proto/Terminals.hpp"

#include "BoundaryConditions.hpp"
#include "InitialConditions.hpp"
#include "LibUFEM.hpp"

namespace cf3 {
  namespace mesh { class Dictionary; }


namespace UFEM {

// Some commonly used proto types that can be used in class declarations
using solver::actions::Proto::FieldVariable;
using solver::actions::Proto::ScalarField;
using solver::actions::Proto::VectorField;
using solver::actions::Proto::PhysicsConstant;

/// LSSAction for UFEM problems, allowing dynamic configuration and providing access to
/// * Linear system solver
/// * Physical model
/// * Mesh used
/// * Region to loop over
class UFEM_API LSSAction : public solver::ActionDirector
{
public: // functions

  /// Contructor
  /// @param name of the component
  LSSAction ( const std::string& name );

  virtual ~LSSAction();

  /// Get the class name
  static std::string type_name () { return "LSSAction"; }

  virtual void execute();

  /// Create the LSS to use
  /// @param matrix_builder Name of the matrix builder to use for the LSS
  math::LSS::System& create_lss(const std::string& matrix_builder = "cf3.math.LSS.TrilinosFEVbrMatrix");

  /// Access to the tag this component uses for finding its solution field
  std::string solution_tag();

  /// Set the tag used to keep track of what field stores the solution to the LSS
  void set_solution_tag(const std::string& tag);

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;

  /// Signals
  void signature_create_lss( common::SignalArgs& node );
  void signal_create_lss( common::SignalArgs& node );

  /// trigger for the dictionary
  void trigger_dictionary();

  /// Trigger for the initial conditions
  void trigger_initial_conditions();

  /// The dictionary to use for field lookups
  Handle<mesh::Dictionary> m_dictionary;

  /// Component that sets initial conditions
  Handle<InitialConditions> m_initial_conditions;

  /// The initial conditions that apply to the current LSSAction
  std::vector< Handle<common::Component> > m_created_initial_conditions;

protected:

  /// Called when the regions are set
  void on_regions_set();

  /// Called when the initial conditions are set
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);

public:
  /// Proto placeholder for the system matrix
  const solver::actions::Proto::SystemMatrix& system_matrix;
  /// Proto placeholder for the right hand side of the system
  const solver::actions::Proto::SystemRHS& system_rhs;
  /// Proto placeholder for dirichlet boundary conditions
  const solver::actions::Proto::DirichletBC& dirichlet;
  /// Proto placeholder for the solution vector
  const solver::actions::Proto::SolutionVector& solution;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_LSSAction_hpp
