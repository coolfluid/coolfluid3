#ifndef CF_Math_LinearFunctor_hh
#define CF_Math_LinearFunctor_hh

////////////////////////////////////////////////////////////////////////////////

#include "RealVector.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class defines a functor that returns a linear RealVector
/// @author Thomas Wuilbaut
template <Uint RESULT_SIZE>
class Math_API LinearFunctor {
public:

  /// Constructor
  explicit LinearFunctor() :
    _result(RESULT_SIZE)
  {
  }

  /// Default destructor
  ~LinearFunctor()
  {
  }

  void setLinearCoef(const Real& value)
  {
    _linearCoef = value;
  }

  void setIndepCoef(const Real& value)
  {
    _indepCoef = value;
  }

  /// @return the size of the result
  CF::Uint size() const
  {
    return RESULT_SIZE;
  }

  /// Overloading of operator()
  CF::RealVector& operator()(const RealVector& value)
  {
    // must assign it always because some client can have changed it meanwhile
    cf_assert(value.size() == _result.size());
    for(Uint i=0; i< _result.size(); ++i)
    {
      _result[i] = _indepCoef + (_linearCoef*value[i]);
    }
    CFout << "Result = " << _result[0] << " using value: " << value[0] << "\n";
    return _result;
  }

  private:

  /// the linear coeficient (b) of the equation result= a+bx
  Real _linearCoef;

  /// the independent coeficient (a) of the equation result= a+bx
  Real _indepCoef;

  /// array storing the temporary solution
  CF::RealVector _result;

}; // end class LinearFunctor

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_LinearFunctor_hh
