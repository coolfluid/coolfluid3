// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_AnalyticalFunction_hpp
#define cf3_Math_AnalyticalFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "fparser/fparser.hh"

#include "math/LibMath.hpp"
#include "math/MatrixTypes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace math {

////////////////////////////////////////////////////////////////////////////////

/// This class represents an analytical function that
/// defines the values for a vector field
/// @author Tiago Quintino
class Math_API AnalyticalFunction {

public: // functions

  /// Empty constructor
  AnalyticalFunction();

  /// Constructor
  AnalyticalFunction( const std::string& function, const std::string& vars);

  /// Destructor
  ~AnalyticalFunction();
  
  /// Define the parameters of the function
  void set_variables(const std::string& vars);

  /// Parse the strings to extract the functions for each line of the vector.
  /// @throw ParsingFailed if there is an error while parsing
  void parse (const std::string& function);

  /// Parse the strings to extract the functions for each line of the vector.
  /// @throw ParsingFailed if there is an error while parsing
  void parse (const std::string& function, const std::string& m_vars);

  /// @return if the AnalyticalFunctionParser has been parsed yet.
  bool is_parsed() const { return m_is_parsed; }

  /// Gets the number of variables
  /// @pre only call this function if already parsed
  /// @returns the number of varibles
  Uint nbvars() const;

  /// The analytical function that is parsed
  const std::string& function() const { return m_function; }

  /// The variables
  const std::string& variables() const { return m_vars; }

  /// Evaluate the Analytical Function given the values of the variables.
  /// @param vars values of the variables to substitute in the function.
  /// @param ret_value the placeholder vector for the result
  template <typename var_t, typename ret_t>
  void evaluate( const var_t& var_values, ret_t& ret_value) const;

  /// Evaluate the Analytical Function given the values of the variables.
  /// This function allows this class to work as a functor.
  /// @param var_values values of the variables to substitute in the function.
  template <typename var_t>
  Real operator()(const var_t& var_values) const;

protected: // helper functions

  /// Clears the m_parser deallocating the memory.
  void clear();

private: // data

  /// flag to indicate if the functions have been parsed
  bool m_is_parsed;

  /// vector holding the names of the variables
  std::string m_vars;

  /// number of variables
  Uint m_nbvars;

  /// a vector of string to hold the functions
  std::string m_function; 

  /// vector holding the parsers, one for each entry in the vector
  boost::shared_ptr<FunctionParser> m_parser;

}; // AnalyticalFunction

////////////////////////////////////////////////////////////////////////////////

template <typename var_t, typename ret_t>
void AnalyticalFunction::evaluate(const var_t& var_values, ret_t& ret_value) const
{
  cf3_assert(m_is_parsed);
  cf3_assert(var_values.size() == m_nbvars);

  // evaluate and store the functions line by line in the vector
  ret_value = m_parser->Eval(&var_values[0]);
}

////////////////////////////////////////////////////////////////////////////////

template <typename var_t>
Real AnalyticalFunction::operator()(const var_t& var_values) const
{
  Real ret_value;
  evaluate(var_values,ret_value);
  return ret_value;
}

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Math_AnalyticalFunction_hpp
