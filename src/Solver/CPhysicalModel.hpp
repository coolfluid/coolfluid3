// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CPhysicalModel_hpp
#define CF_Solver_CPhysicalModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

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
  Uint dimensions() const;
  
  /// @return the number of degrees of freedom (DOFs), i.e. the number of components of the state vector (the number of scalars needed to represent
  /// the solution at a single node)
  Uint nb_dof() const;
  
  /// @return the number of degrees of nodes in the mesh.
  Uint nb_nodes() const;

  /// True if the variable with the given name is part of the solution state
  bool is_state_variable(const std::string& var_name) const;
  
  /// Get the offset in the state for the variable with the supplied name, i.e. if the variables are ordered u, v, p in the system, the offset for p is 2.
  Uint offset(const std::string& var_name) const;
  
  /// Register a variable. The order of registration also determines the storage order for the equations in the physical model. If a variable with the same
  /// name was already registered, nothing is changed.
  /// @param name Unique name by which this value is referred.
  /// @param symbol Short name for the variable.  By default, the variable will be named like this in the field
  /// The given string is also linked to an option that gets created
  /// @param field_name Default field name
  /// The given string is also linked to an option that gets created
  /// @param var_type Type of the variable
  /// @param is_equation_var True if the variable represents a state, i.e. something that is solved for
  void register_variable(const std::string& name, std::string& symbol, std::string& field_name, const VariableTypesT var_type, const bool is_state);
  
  /// Create the fields for all registered variables
  void create_fields();
  
  /// Return the type of the variable with the given key
  VariableTypesT variable_type(const std::string& name) const;
  
  /// @return the physical model type
  /// @todo make this a pure virtual function
  std::string type() const;

  const State& solution_state() const;
  
  /// Stores the URIs of fields used for state variables in fieldlist. Useful for obtaining fields that are written using a mesh writer
  void state_fields(std::vector<Common::URI>& fieldlist) const;

private: // data

  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
}; // CPhysicalModel

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CPhysicalModel_hpp
