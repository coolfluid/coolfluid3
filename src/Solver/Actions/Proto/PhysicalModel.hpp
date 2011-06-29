// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_PhysicalModel_hpp
#define CF_Solver_Actions_Proto_PhysicalModel_hpp

#include "Common/CF.hpp"
#include "Math/MatrixTypes.hpp"

#include "Terminals.hpp"

namespace CF {
  namespace Mesh { class CMesh; }
  namespace Common { class PropertyList; }
namespace Solver {
namespace Actions {
namespace Proto {

/// Simplified version of a physical model, intended for internal use by proto
class PhysicalModel
{
public:
  /// Available variable types
  enum VariableTypesT
  {
    SCALAR = 0,
    VECTOR = 1
  };

  PhysicalModel();

  /// Number of degrees of freedom for the solved system (includes only the variables in the equation). It is the number of scalars needed to represent
  /// the solution at a single node. Calculated during the create_fields step.
  Uint nb_dofs() const;

  /// True if the variable with the given internal name is part of the solution state
  bool is_equation_variable(const std::string& var_name) const;

  /// Get the in the linear system for the variable with the supplied name, i.e. if the variables are ordered u, v, p in the system, the offset for p is 2.
  Uint offset(const std::string& var_name) const;

  /// Register a new variable with the physical model
  /// @param var_name Internal name of the variable
  /// @param var_type The type of variable
  /// @param is_equation_var Indicate if the variable belongs to the states representing the solution
  void register_variable(const std::string& var_name, const VariableTypesT var_type, const bool is_equation_var);

  /// Directly register variables (when not using component wrappers)
  template<Uint I, typename T>
  void register_variable(const MeshTerm<I, T>& var, const bool is_equation_var)
  {
    register_variable(boost::proto::value(var).variable_value, is_equation_var);
  }

  /// Create the fields, based on the properties that are linked to each variable and calculate the number of DOFs.
  void create_fields(Mesh::CMesh& mesh, const Common::OptionList& options);

  /// Create the fields, based on the variables that were registered directly
  void create_fields(Mesh::CMesh& mesh);

  /// Update the solution fields
  /// @param solution_mesh Mesh containing the solution
  /// @param solution Vector with the difference between the new solution and the old solution
  void update_fields(Mesh::CMesh& solution_mesh, const RealVector& solution);

  /// Clear the variables from this physical model and set the nb_dofs to 0
  void clear();

private:

  /// Offset of each equation variable, i.e. in V (vector of u and v) and p, V has offset 0, and p has offset 2 when the order is uvp in the global system
  std::map<std::string, Uint> m_variable_offsets;
  /// Ordered list of the equation variables
  std::vector<std::string> m_equation_variables;
  /// Type of each variable
  typedef std::map<std::string, VariableTypesT> VarTypesT;
  VarTypesT m_variable_types;
  /// Degrees of freedom, i.e. the number of scalars that are defined at a single node
  Uint m_nb_dofs;

  void register_variable(const ScalarField& var, const bool is_equation_var);
  void register_variable(const VectorField& var, const bool is_equation_var);
  void get_names(const Common::OptionList& options);

  /// Storage for field and variable names
  std::map<std::string, std::string> m_field_names;
  std::map<std::string, std::string> m_variable_names;

  /// Data used to update the solution
  std::vector<std::string> m_solution_fields;
  std::vector<std::string> m_solution_variables;
  std::vector<Uint> m_solution_sizes;
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_PhysicalModel_hpp
