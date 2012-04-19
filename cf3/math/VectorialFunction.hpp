// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_VectorialFunction_hpp
#define cf3_Math_VectorialFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "fparser/fparser.hh"

#include "common/BasicExceptions.hpp"

#include "math/LibMath.hpp"
#include "math/MatrixTypes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace math {

////////////////////////////////////////////////////////////////////////////////

/// This class represents an analytical function that
/// defines the values for a vector field
/// @author Tiago Quintino
class Math_API VectorialFunction {

public: // functions

  /// Variable storage
  typedef std::vector<Real> VariablesT;

  /// Empty constructor
  VectorialFunction();

  /// Constructor
  VectorialFunction( const std::string& functions, const std::string& vars);

  /// Destructor
  ~VectorialFunction();

//  /// Evaluate the Vectorial Function given the values of the variables.
//  /// @param vars values of the variables to substitute in the function.
//  /// @param ret_value the placeholder vector for the result
//  void evaluate (const VariablesT& var_values, RealVector& ret_value) const;

//  /// Evaluate the Vectorial Function given the values of the variables.
//  /// @param vars values of the variables to substitute in the function.
//  /// @param ret_value the placeholder vector for the result
//  void evaluate (const RealVector& var_values, RealVector& ret_value) const;

  /// Evaluate the Vectorial Function given the values of the variables.
  /// @param vars values of the variables to substitute in the function.
  /// @param ret_value the placeholder vector for the result
  template <typename var_t, typename ret_t>
  void evaluate( const var_t& var_values, ret_t& ret_value) const;

  /// Evaluate the Vectorial Function given the values of the variables
  /// and return it in the stored result. This function allows this class to work
  /// as a functor.
  /// @param var_values values of the variables to substitute in the function.
  RealVector& operator()(const VariablesT& var_values);

  /// Evaluate the Vectorial Function given the values of the variables
  /// and return it in the stored result. This function allows this class to work
  /// as a functor.
  /// @param var_values values of the variables to substitute in the function.
  RealVector& operator()(const RealVector& var_values);

  /// @return if the VectorialFunctionParser has been parsed yet.
  bool is_parsed() const { return m_is_parsed; }

  /// sets the function strings to be parsed
  void functions( const std::vector<std::string>& functions );

  /// sets the function strings to be parsed
  void functions( const std::string& functions );

  /// sets the variable strings to be parsed
  void variables( const std::vector<std::string>& vars );

  /// sets the variable strings to be parsed
  void variables( const std::string& vars );

  /// Parse the strings to extract the functions for each line of the vector.
  /// @throw ParsingFailed if there is an error while parsing
  void parse ();

  /// Gets the number of variables
  /// @pre only call this function if already parsed
  /// @returns the number of varibles
  Uint nbvars() const;

  /// Gets the number of variables
  /// @pre only call this function if already parsed
  /// @returns the number of varibles
  Uint nbfuncs() const;

protected: // helper functions

  /// Clears the m_parsers deallocating the memory.
  void clear();

private: // data

  /// flag to indicate if the functions have been parsed
  bool m_is_parsed;

  /// vector holding the names of the variables
  std::string m_vars;

  /// number of variables
  Uint m_nbvars;

  /// a vector of string to hold the functions
  std::vector<std::string> m_functions;

  /// vector holding the parsers, one for each entry in the vector
  std::vector<FunctionParser*> m_parsers;

  /// storage of the result for using the class as functor
  RealVector m_result;

}; // VectorialFunction

////////////////////////////////////////////////////////////////////////////////

template <typename var_t, typename ret_t>
void VectorialFunction::evaluate(const var_t& var_values, ret_t& ret_value) const
{
  cf3_assert(m_is_parsed);
  cf3_assert(var_values.size() == m_nbvars);

  // evaluate and store the functions line by line in the vector
  std::vector<FunctionParser*>::const_iterator parser = m_parsers.begin();
  std::vector<FunctionParser*>::const_iterator end = m_parsers.end();
  for(Uint i=0 ; parser != end ; ++parser, ++i )
  {
    // It is possible this function signals a FloatingPointException (FPE)
    ret_value[i] = (*parser)->Eval(&var_values[0]);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Math_VectorialFunction_hpp
