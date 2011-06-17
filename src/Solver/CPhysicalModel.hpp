// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CPhysicalModel_hpp
#define CF_Solver_CPhysicalModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/weak_ptr.hpp>

#include "Common/Component.hpp"
#include "Solver/State.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
  namespace Mesh { class CMesh; }
namespace Common {

template<class T>
class OptionComponent;
}

namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Component providing information about the physics, i.e. number of equations, variables, ...
/// @author Bart Janssens
class Solver_API CPhysicalModel : public Common::Component {

public: //typedefs

  typedef boost::shared_ptr<CPhysicalModel> Ptr;
  typedef boost::shared_ptr<CPhysicalModel const> ConstPtr;

  /// Available variable types
  enum VariableTypesT
  {
    SCALAR = 0,
    VECTOR = 1
  };
  
public: // functions

  /// Contructor
  /// @param name of the component
  CPhysicalModel ( const std::string& name );

  /// Virtual destructor
  virtual ~CPhysicalModel();

  /// Get the class name
  static std::string type_name () { return "CPhysicalModel"; }

  //////////////////////////////////
  // CPhysicalModel specific
  /////////////////////////////////
  
  /// @return dimensionality of the problem, which is
  ///         the number of spatial coordinates used in the PDEs
  Uint dimensions() const { return m_dim; }
  
  /// @return the number of degrees of freedom (DOFs), i.e. the number of components of the state vector (the number of scalars needed to represent
  /// the solution at a single node)
  Uint nb_dof() const { return m_nbdofs; }

  /// True if the variable with the given name is part of the solution state
  bool is_state_variable(const std::string& var_name) const;
  
  /// Get the offset in the state for the variable with the supplied name, i.e. if the variables are ordered u, v, p in the system, the offset for p is 2.
  Uint offset(const std::string& var_name) const;
  
  /// Register a variable. The order of registration also determines the storage order for the equations in the physical model.
  /// @param name Unique name by which this value is referred. By default, this is also the name of the field it will be stored in
  /// @param symbol Short name for the variable.  By default, the variable will be named like this in the field
  /// @param var_type Type of the variable
  /// @param is_equation_var True if the variable represents a state, i.e. something that is solved for
  void register_variable(const std::string& name, const std::string& symbol, const VariableTypesT var_type, const bool is_state);

  /// Create the fields for all registered variables
  void create_fields();
  
  /// Convenience method to set the mesh
  void set_mesh(Mesh::CMesh& mesh);

  /// @return the physical model type
  /// @todo make this a pure virtual function
  std::string type() const { return m_type; }


  const State& solution_state() const { return *m_solution_state; }

private: // functions

  void build_solution_state();
  
  /// Recreate fields when the mesh changes, and update the dimension
  void trigger_mesh();

private: // data

  /// type of the physcial model
  std::string m_type;

  /// dimensionality of physics
  Uint m_dim;

  /// number of degrees of freedom
  Uint m_nbdofs;

  State::Ptr m_solution_state;
  
  /// Option for referring to the mesh
  boost::weak_ptr< Common::OptionComponent<Mesh::CMesh> > m_mesh_option;
  
  /// Offset of each equation variable, i.e. in V (vector of u and v) and p, V has offset 0, and p has offset 2 when the order is uvp in the global system
  std::map<std::string, Uint> m_variable_offsets;
  
  /// Ordered list of the equation variables
  std::vector<std::string> m_state_variables;
  
  /// Type of each variable
  typedef std::map<std::string, VariableTypesT> VarTypesT;
  VarTypesT m_variable_types;
  
  /// Storage for field and variable names
  std::map<std::string, std::string> m_field_names;
  std::map<std::string, std::string> m_variable_names;
  
  /// Data used to update the solution
  std::vector<std::string> m_solution_fields;
  std::vector<std::string> m_solution_variables;
  std::vector<Uint> m_solution_sizes;
}; // CPhysicalModel

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CPhysicalModel_hpp
