#ifndef COOLFluiD_MathTools_ConstantFunctor_hh
#define COOLFluiD_MathTools_ConstantFunctor_hh

//////////////////////////////////////////////////////////////////////////////

#include "RealVector.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

/// This class defines a functor that returns a compile time constant RealVector
/// @author Tiago Quintino
template <CFuint RESULT_SIZE>
class ConstantFunctor {
public:

  /// Constructor
  explicit ConstantFunctor() :  _result(RESULT_SIZE)  {}

  /// Default destructor
  ~ConstantFunctor() {}

  void setValue(const CFreal& value) {  _value = value; }

  /// @return the size of the result
  COOLFluiD::CFuint size() const {  return RESULT_SIZE; }

  /// Overloading of operator()
  COOLFluiD::RealVector& operator()()
  {
    // must assign it always because some client can have changed it meanwhile
    _result = _value;
    return _result;
  }

  private:

  /// the value to return
  CFreal _value;

  /// array storing the temporary solution
  COOLFluiD::RealVector _result;

}; // end class ConstantFunctor

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_ConstantFunctor_hh
