// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_VariablesDescriptor_hpp
#define CF_Math_VariablesDescriptor_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>

#include "Common/Component.hpp"

#include "Math/LibMath.hpp"

namespace CF {

namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// Holds information related to a set of variables that are stored in the same field.
/// Provides options:
/// For each variable, an option composed of lowercase(internal variable name) + _variable_name
/// dimensions: Dimension of the problem (i.e. number of spatial coordinates used)
/// @author Bart Janssens
/// @author Tiago Quintino
class Math_API VariablesDescriptor : public Common::Component {

public: //typedefs

  typedef boost::shared_ptr<VariablesDescriptor> Ptr;
  typedef boost::shared_ptr<VariablesDescriptor const> ConstPtr;

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
    enum Type { INVALID=-1, SCALAR=1, VECTOR=-2, TENSOR=-3};

    struct Math_API Convert : public Common::EnumT< Dimensionalities >
    {
      /// constructor where all the converting maps are built
      Convert();

      /// get the unique instance of the converter class
      static Convert& instance() { static Convert instance; return instance; }
    };
  };
  
  /// Total size of the array of scalars to hold each variable, i.e. the required row size for the CField data table.
  /// Throws if dimensions is not set
  Uint size() const;
  
  /// Size of a variable
  /// @param name Internal name of the variable
  /// @returns the actual size of the variable
  /// Throws if dimensions is not set
  Uint size(const std::string& name);
  
  /// Offset in the field for a variable, i.e. the start index of the variable in the CField row.
  /// Throws if dimensions is not set
  /// @param name Internal name of the variable
  Uint offset(const std::string& name);
  
  /// Return the user-defined name of a variable
  /// @param name Internal name of the variable
  const std::string& user_variable_name(const std::string& name) const;
  
  /// Setup variables acording to a CField string description
  void set_variables(const std::string& description);
  
  /// Get the string description for all the variables
  std::string description() const;
  
  /// Append a variable to the back of the list. Does nothing if the variable with the given name already existed
  /// @param name Internal name of the variable
  void push_back(const std::string& name, const Dimensionalities::Type type);
  
  //@} End Variable management

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
                        
}; // VariablesDescriptor

////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_VariablesDescriptor_hpp
