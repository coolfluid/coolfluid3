#ifndef CF_Math_ConstantFunctor_hh
#define CF_Math_ConstantFunctor_hh

////////////////////////////////////////////////////////////////////////////////

#include "RealVector.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class defines a functor that returns a compile time constant RealVector
/// @author Tiago Quintino
template <Uint RESULT_SIZE>
class ConstantFunctor {
public:

  /// Constructor
  explicit ConstantFunctor() :  _result(RESULT_SIZE)  {}

  /// Default destructor
  ~ConstantFunctor() {}

  void setValue(const Real& value) {  _value = value; }

  /// @return the size of the result
  CF::Uint size() const {  return RESULT_SIZE; }

  /// Overloading of operator()
  CF::RealVector& operator()()
  {
    // must assign it always because some client can have changed it meanwhile
    _result = _value;
    return _result;
  }

  private:

  /// the value to return
  Real _value;

  /// array storing the temporary solution
  CF::RealVector _result;

}; // end class ConstantFunctor

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_ConstantFunctor_hh
