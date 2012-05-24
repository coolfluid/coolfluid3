// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_VariablesDescriptor_hpp
#define cf3_Math_VariablesDescriptor_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "common/Component.hpp"

#include "math/LibMath.hpp"

namespace cf3 {

namespace math {

////////////////////////////////////////////////////////////////////////////////

/// Holds information related to a set of variables that are stored in the same field.
/// Provides options:
/// For each variable, an option composed of lowercase(internal variable name) + _variable_name
/// dimensions: Dimension of the problem (i.e. number of spatial coordinates used)
/// @author Bart Janssens
/// @author Tiago Quintino
class Math_API VariablesDescriptor : public common::Component {

public: // functions

  /// constructor
  /// @param name of the component
  VariablesDescriptor ( const std::string& name );

  /// virtual destructor
  virtual ~VariablesDescriptor();

  /// Get the class name
  static std::string type_name () { return "VariablesDescriptor"; }

  /// @name VariablesDescriptor interface
  /// Functions to manage the list of variables that is stored in a field
  //@{

  /// Possible dimensions for variables:
  /// SCALAR
  /// VECTOR: dimension of the problem
  /// TENSOR: rank 2 tensor with the dimension of the problem
  struct Math_API Dimensionalities
  {
    enum Type { INVALID=-1, SCALAR=1, VECTOR=-2, TENSOR=-3, ARRAY=-4};

    struct Math_API Convert : public common::EnumT< Dimensionalities >
    {
      /// constructor where all the converting maps are built
      Convert();

      /// get the unique instance of the converter class
      static Convert& instance() { static Convert instance; return instance; }
    };
  };

  /// The number of variables
  /// This is not the same as the total number of scalars. Some variables can be vectors or tensors
  /// @return the number of variables
  Uint nb_vars() const;

  /// Total size of the array of scalars to hold each variable, i.e. the required row size for the Field data table.
  /// @throws SetupError if dimensions is not set
  Uint size() const;

  /// Size of a variable
  /// @param name Internal name of the variable
  /// @returns the actual size of the variable
  /// @throws SetupError if dimensions is not set
  Uint size(const std::string& name) const;

  /// Offset in the field for a variable, i.e. the start index of the variable in the Field row.
  /// @throws SetupError if dimensions is not set
  /// @param name Internal name of the variable
  Uint offset(const std::string& name) const;

  /// Offset in the field for a variable, i.e. the start index of the variable in the Field row.
  /// @throws SetupError if dimensions is not set
  /// @param name Internal name of the variable
  Uint offset(const Uint var_nb) const;

  /// Return if the variable with a given name is found in this description
  /// @param name Internal name of the variable
  /// @return true if found, false if not found
  bool has_variable(const std::string& name) const;

  /// The dimensionality of the variable with the given name
  /// @param name Internal name of the variable
  Dimensionalities::Type dimensionality(const std::string& name) const;

  /// Find the variable number for a given variable name.
  /// @throws ValueNotFound if the variable is not found
  /// @param name Internal name of the variable
  /// @return variable number
  Uint var_number(const std::string& name) const;

  /// Get the variable size for a given variable name.
  /// @throws ValueNotFound if the variable is not found
  /// @param name Internal name of the variable
  /// @return variable length
  Uint var_length(const std::string& name) const;

  /// Get the variable size for a given variable name.
  /// @throws ValueNotFound if the variable is not found
  /// @param name Internal name of the variable
  /// @return variable length
  Uint var_length(const Uint var_nb) const;

  /// Return the user-defined name of a variable
  /// @param name Internal name of the variable
  const std::string& user_variable_name(const std::string& name) const;

  /// Return the user-defined name of a variable
  /// @param name Internal name of the variable
  const std::string& user_variable_name(const Uint var_nb) const;

  /// Return the internal unique name of a variable
  /// @param var_nb The variable number
  const std::string& internal_variable_name(const Uint var_nb) const;

  /// Setup variables acording to a Field string description
  void set_variables(const std::string& description);

  /// Setup variables together with their dimension
  void set_variables(const std::string& description, const Uint dimension);

  /// Get the string description for all the variables
  std::string description() const;

  /// Append a variable to the back of the list. Does nothing if the variable with the given name already existed
  /// @param name Internal name of the variable
  void push_back(const std::string& name, const Dimensionalities::Type type);
  void push_back(const std::string& name, const Uint nb_vars);

  void prefix_variable_names(const std::string& prefix);

  //@} End Variable management

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;

}; // VariablesDescriptor

////////////////////////////////////////////////////////////////////////////////

Math_API std::ostream& operator<< ( std::ostream& os, const VariablesDescriptor::Dimensionalities::Type& in );
Math_API std::istream& operator>> ( std::istream& is, VariablesDescriptor::Dimensionalities::Type& in );

} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Math_VariablesDescriptor_hpp
