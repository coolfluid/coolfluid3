// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BoundaryConditions_hpp
#define cf3_UFEM_BoundaryConditions_hpp

#include "common/ActionDirector.hpp"
#include "common/OptionURI.hpp"

#include "solver/Solver.hpp"

#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// BoundaryConditions for UFEM problems
class UFEM_API BoundaryConditions : public common::ActionDirector
{

public: // functions

  /// Contructor
  /// @param name of the component
  BoundaryConditions ( const std::string& name );

  virtual ~BoundaryConditions();

  /// Get the class name
  static std::string type_name () { return "BoundaryConditions"; }

  /// Create constant dirichlet BC
  /// @param region_name Name of the boundary region. Must be unique in the problem region
  /// @param variable_name Name of the variable for which to set the BC
  Handle<common::Action> add_constant_bc(const std::string& region_name, const std::string& variable_name);

  /// Create constant dirichlet BC
  /// @param region_name Name of the boundary region. Must be unique in the problem region
  /// @param variable_name Name of the variable for which to set the BC
  /// @param default_value Default value
  Handle<common::Action> add_constant_bc(const std::string& region_name, const std::string& variable_name, const boost::any default_value);

  /// Set only one component of a vector variable
  Handle<common::Action> add_constant_component_bc(const std::string& region_name, const std::string& variable_name, const Uint component_idx, const Real default_value = 0.);

  /// Create a dirichlet BC that can be set using a user-supplied function
  /// @param region_name Name of the boundary region. Must be unique in the problem region
  /// @param variable_name Name of the variable for which to set the BC
  Handle<common::Action> add_function_bc(const std::string& region_name, const std::string& variable_name);
  
  /// Create an action to be used as a BC, with the given builder name
  Handle<common::Action> create_bc_action(const std::string& region_name, const std::string& builder_name);

  /// Signal to create a constant BC and add it to the sequence of executed actions
  void signal_create_constant_bc(common::SignalArgs& node);
  void signal_create_function_bc(common::SignalArgs& node);
  void signal_create_constant_component_bc(common::SignalArgs& node);
  void signal_create_bc_action(common::SignalArgs& node);

  void set_solution_tag(const std::string& solution_tag);

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BoundaryConditions_hpp
