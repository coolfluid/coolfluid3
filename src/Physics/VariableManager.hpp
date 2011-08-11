// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_VariableManager_hpp
#define CF_Physics_VariableManager_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "Common/Component.hpp"

#include "Math/MatrixTypes.hpp"

#include "Physics/PhysModel.hpp"

namespace CF {

namespace Physics {

////////////////////////////////////////////////////////////////////////////////

/// Manage the variables needed in a model
/// @author Bart Janssens
/// @author Tiago Quintino
class Physics_API VariableManager : public Common::Component {

public: //typedefs

  typedef boost::shared_ptr<VariableManager> Ptr;
  typedef boost::shared_ptr<VariableManager const> ConstPtr;

public: // functions

  /// constructor
  /// @param name of the component
  VariableManager ( const std::string& name );

  /// virtual destructor
  virtual ~VariableManager();

  /// Get the class name
  static std::string type_name () { return "VariableManager"; }

  /// @name Variable management
  /// Generic functionality to manage the variables that are used in a solver, including options to control field and variable names
  //@{
  
  /// Available variable types
  enum VariableTypesT
  {
    SCALAR = 0,
    VECTOR = 1
  };
  
  /// Register a variable. The order of registration also determines the storage order for the equations in the physical model.
  /// The symbol and field_name parameters are linked to options that allow user control.If a variable with the same
  /// name was already registered, nothing is changed, except for the option linking.
  /// @param name Unique name by which this value is referred (internal to the model).
  /// @param symbol Short name for the variable.  By default, the variable will be named like this in the field
  /// The given string is also linked to an option that gets created, allowing the user to change the name of this variable
  /// @param field_name Default field name
  /// The given string is also linked to an option that gets created, allowing the user to change the name of the field
  /// @param var_type Type of the variable
  /// @param is_state True if the variable represents a state, i.e. something that is solved for
  void register_variable(const std::string& var_name, std::string& symbol, std::string& field_name, const VariableTypesT var_type, const bool is_state);
  
  /// True if the variable with the given name is part of the solution state
  bool is_state_variable(const std::string& var_name) const;
  
  /// Get the offset in the state for the variable with the supplied name, i.e. if the variables are ordered u, v, p in the system, the offset for p is 2.
  Uint offset(const std::string& var_name) const;
  
  /// @return the number of degrees of freedom (DOFs), i.e. the number of components of the state vector (the number of scalars needed to represent
  /// the solution at a single node)
  Uint nb_dof() const;
  
  /// Return the type of the variable with the given key
  VariableTypesT variable_type(const std::string& var_name) const;
  
  /// Stores the names of fields used for state variables in fieldlist.
  void state_fields(std::vector< std::string >& fieldlist) const;
  
  /// Store the fields and their varible in the supplied map
  /// @param fields map to fill, where the key will be the field name and the value the string specifying the variables of the field,
  /// using the Field protocol
  void field_specification(std::map<std::string, std::string>& fields);
  
  //@} End Variable management

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
                        
}; // VariableManager

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Physics_VariableManager_hpp
