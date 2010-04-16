#ifndef CF_Math_VectorialFunction_hh
#define CF_Math_VectorialFunction_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/BasicExceptions.hpp"
#include "Common/StringOps.hpp"
#include "Math/RealVector.hpp"
#include "Math/FunctionParser.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math { 

////////////////////////////////////////////////////////////////////////////////

/// This class represents a Function that defines the values
/// for a vector field.
/// @author Tiago Quintino
class Math_API VectorialFunction {

public: // functions

  /// Default constructor without arguments
  VectorialFunction();

  /// Default destructor
  ~VectorialFunction();

  /// Evaluate the Vectorial Function given the values of the variables.
  /// @param vars values of the variables to substitute in the function.
  /// @param value the placeholder vector for the result
  void evaluate(const RealVector& varValue, RealVector& value) const;

  /// Evaluate the Vectorial Function given the values of the variables
  /// and return it in the stored result. This function allows this class to work
  /// as a functor.
  /// @param vars values of the variables to substitute in the function.
  RealVector& operator()(const RealVector& varValue);

  /// @return if the VectorialFunctionParser has been parsed yet.
  bool isParsed() const
  {
    return m_isParsed;
  }

  /// Set Function
  void setFunctions(const std::vector<std::string>& functions);

  /// Set Vars
  void setVariables(const std::vector<std::string>& vars);

  /// Parse the strings to extract the functions for each line of the vector.
  /// @param functions vector of string describing the functions for each line.
  /// @param vars the variables to be taken into account.
  /// @throw ParsingFailed if there is an error while parsing
  void parse();

  /// Gets the number of variables
  /// @pre only call this function if already parsed
  /// @returns the number of varibles
  Uint getNbVars() const
  {
    cf_assert ( isParsed() );
    return m_nbVars;
  }

  /// Gets the number of variables
  /// @pre only call this function if already parsed
  /// @returns the number of varibles
  Uint getNbFuncs() const
  {
    cf_assert ( isParsed() );
    return m_functions.size();
  }

protected: // helper functions

  /// Clears the m_parsers deallocating the memory.
  void clear();

private: // data

  /// flag to indicate if the functions have been parsed
  bool m_isParsed;

  /// vector holding the names of the variables
  std::string m_vars;

  /// number of variables
  Uint m_nbVars;

  /// a vector of string to hold the functions
  std::vector<std::string> m_functions;

  /// vector holding the parsers, one for each entry in the vector
  std::vector<FunctionParser*> m_parsers;

  /// storage of the result for using the class as functor
  RealVector m_result;

}; // end of class VectorialFunction

////////////////////////////////////////////////////////////////////////////////

  } // namespace Framework

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Framework_VectorialFunction_hh
